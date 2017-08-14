/* Copyright 2006 Nokia Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Symbian
#include <cntdb.h>
#include <cntfldst.h>
#include <cntitem.h>
#include <logcli.h>
#include <logview.h>
#include <logwrap.h>
#include <logwrap.hrh>

#include "symbian_mod_utils.h"

// Apache httpd
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"


/* Logs Module - displays the logs on a Symbian phone.
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /logs>
 * SetHandler logs
 * </Location>
 *
 * You may want to protect this location by password or domain so no one
 * else can look at it.  Then you can access the logs an URL like:
 *
 * http://your_server_name/logs
 *
 * /logs   - Returns all logs as a clickable list.
 *
 */


namespace
{
    struct Entry
        {
        const char* pHeading;
        const char* pDefault;
        const char* pLink;
        TInt        idList;
        } entries[] = 
        {
        {
        "Missed Calls",
        "(no missed calls)",
        "<a href=\"logs?missed\">Missed Calls</a>",
        KLogRecentMissedCalls
        },
        {
        "Received Calls",
        "(no received calls)",
        "<a href=\"logs?incoming\">Received Calls</a>",
        KLogRecentIncomingCalls
        },
        {
        "Dialled Numbers",
        "(no dialled numbers)",
        "<a href=\"logs?outgoing\">Dialled numbers</a>",
        KLogRecentOutgoingCalls
        }
        };
    
    const TInt KSizeEntries = sizeof(entries)/sizeof(entries[0]);
    
    const TInt KIndexMissed   = 0;
    const TInt KIndexIncoming = 1;
    const TInt KIndexOutgoing = 2;
    
    const TInt KMaxIntChars = 10; // strlen("4294967296")
}


namespace
{
    NONSHARABLE_CLASS(CLogEventGenerator) : public CActive
        {
    public:
        ~CLogEventGenerator();
        
        static CLogEventGenerator* NewLC(CLogViewRecent* apLogView,
                                         TLogRecentList  aList,
                                         const TDesC8*   apDefault,
                                         request_rec*    r);
        
        void GenerateL();

    private:
        void RunL();
        
        void DoCancel();
        
        void DoRunL();

    private:
        CLogEventGenerator(CLogViewRecent* apLogView,
                           TLogRecentList  aList,
                           const TDesC8*   apDefault,
                           request_rec*    r);

        void ConstructL();

        void InitializeL();
        void StartL();
        void IterateL();
        void Stop();
        
        enum TState
            {
            EIdle,
            EInitializing,
            EStarting,
            EIterating
            };
        
        void SetState(TState aState);

        void GenerateEntryL(const CLogEvent& aEvent);
        void GenerateCallEntryL(const CLogEvent& aEvent);
        
    private:
        CLogViewRecent&       iLogView;
        TLogRecentList        iList;
        const TDesC8&         iDefault;
        request_rec*          r;
        CActiveSchedulerWait* ipScheduler;
        CContactDatabase*     ipDb;
        TState                iState;
        };


    CLogEventGenerator::CLogEventGenerator(CLogViewRecent* apLogView,
                                           TLogRecentList  aList,
                                           const TDesC8*   apDefault,
                                           request_rec*    rr)
        : CActive(CActive::EPriorityStandard),
          iLogView(*apLogView),
          iList(aList),
          iDefault(*apDefault),
          r(rr),
          iState(EIdle)
        {
        CActiveScheduler::Add(this);
        }


    CLogEventGenerator::~CLogEventGenerator()
        {
        delete ipDb;
        delete ipScheduler;
        }


    CLogEventGenerator* CLogEventGenerator::NewLC(CLogViewRecent* apLogView,
                                                  TLogRecentList  aList,
                                                  const TDesC8*   apDefault,
                                                  request_rec*    r)
        {
        CLogEventGenerator
            *pThis = new (ELeave) CLogEventGenerator(apLogView, 
                                                     aList,
                                                     apDefault,
                                                     r);

        CleanupStack::PushL(pThis);

        pThis->ConstructL();

        return pThis;
        }


    void CLogEventGenerator::ConstructL()
        {
        ipScheduler = new (ELeave) CActiveSchedulerWait;
        ipDb = CContactDatabase::OpenL();
        }


    void CLogEventGenerator::GenerateL()
        {
        RunL();
        }


    void CLogEventGenerator::RunL()
        {
        TRAPD(error, DoRunL());
        
        if (error != KErrNone)
            Stop();
        }


    void CLogEventGenerator::DoRunL()
        {
        switch (iState)
            {
            case EIdle:
                InitializeL();
                break;
                
            case EInitializing:
                StartL();
                break;
                
            case EStarting:
                IterateL();
                break;

            case EIterating:
                IterateL();
                break;
            }
        }


    void CLogEventGenerator::DoCancel()
        {
        iLogView.Cancel();
        }


    void CLogEventGenerator::InitializeL()
        {
        if (iLogView.SetRecentListL(iList, iStatus))
            {
            SetState(EInitializing);
            SetActive();
            
            ipScheduler->Start();
            }
        else
            {
            ap_rputd(iDefault, r);
            ap_rputs("\n", r);
            ap_rputs("<hr>\n", r);
            }
        }


    void CLogEventGenerator::StartL()
        {
        if (iLogView.FirstL(iStatus))
            {
            SetState(EStarting);
            SetActive();
            }
        else
            Stop();
        }


    void CLogEventGenerator::IterateL()
        {
        GenerateEntryL(iLogView.Event());
        
        if (iLogView.NextL(iStatus))
            {
            SetState(EIterating);
            SetActive();
            }
        else
            Stop();
        }


    void CLogEventGenerator::Stop()
        {
        ipScheduler->AsyncStop();
        
        SetState(EIdle);
        }


    void CLogEventGenerator::SetState(TState aState)
        {
        iState = aState;
        }


    void CLogEventGenerator::GenerateEntryL(const CLogEvent& aEvent)
        {
        switch (aEvent.EventType().iUid)
            {
            case KLogCallEventType:
                GenerateCallEntryL(aEvent);
                break;
                
            case KLogDataEventType:
            case KLogFaxEventType:
            case KLogShortMessageEventType:
            case KLogMailEventType:
            case KLogTaskSchedulerEventType:
                break;
            }
        }


    namespace
    {
        TBool PutStringL(const TPtrC& aString, request_rec* r)
            {
            TBool
                rc = EFalse;
            
            if (aString.Length() != 0)
                {
                ap_rputdL(aString, r);
                
                rc = ETrue;
                }
            
            return rc;
            }
        
        
        TBool PutFieldL(const CContactItemFieldSet& aFields, 
                        TInt                        aIndex,
                        request_rec*                r)
            {
            TPtrC
                field = aFields[aIndex].TextStorage()->Text();
            
            return PutStringL(field, r);
            }


        TBool PutContactL(CContactItem& aContact, request_rec* r)
            {
            TBool
                rc = EFalse;
            CContactItemFieldSet
                &fields = aContact.CardFields();
            TInt
                iFamily = fields.Find(KUidContactFieldFamilyName),
                iGiven  = fields.Find(KUidContactFieldGivenName),
                iNumber = fields.Find(KUidContactFieldPhoneNumber);
            
            if ((iFamily != KErrNotFound)   || 
                (iGiven != KErrNotFound) || 
                (iNumber != KErrNotFound))
                {
                ap_rputs("<a href=\"contacts?", r);
                
                TBuf8<KMaxIntChars>
                    id;
                
                id.Num(static_cast<TInt>(aContact.Id()));
                
                ap_rwrite(id.Ptr(), id.Length(), r);
                ap_rputs("\">", r);

                if (iFamily != KErrNotFound)
                    {
                    if (PutFieldL(fields, iFamily, r))
                        {
                        iNumber = KErrNotFound;
                        
                        if (iGiven != KErrNotFound)
                            ap_rputs(" ", r);
                        }
                    }
                
                if (iGiven != KErrNotFound)
                    {
                    if (PutFieldL(fields, iGiven, r))
                        iNumber = KErrNotFound;
                    }
                
                if (iNumber != KErrNotFound)
                    PutFieldL(fields, iNumber, r);

                ap_rputs("</a>\n", r);

                rc = ETrue;
                }

            return rc;
            }


        void PutTime(const CLogEvent& aEvent, request_rec* r)
            {
            ap_rputs("<br>\n", r);
            
            const TDateTime
                time = aEvent.Time().DateTime();
            
            ap_rprintf(r, 
                       "%d/%d/%d %d:%d\n",
                       time.Day(),
                       time.Month(),
                       time.Year(),
                       time.Hour(),
                       time.Minute());
                       
            ap_rputs("<hr>\n", r);
            }
        

        void PutNumber(const CLogEvent& aEvent, request_rec* r)
            {
            PutStringL(aEvent.Number(), r);
            }
    }

    
    void CLogEventGenerator::GenerateCallEntryL(const CLogEvent& aEvent)
        {
        TContactItemId
            id = aEvent.Contact();
        
        if (id != -1)
            {
            CContactItem
            *pContact = ipDb->ReadContactL(aEvent.Contact());
            
            CleanupStack::PushL(pContact);
        
            if (PutContactL(*pContact, r))
                PutTime(aEvent, r);

            CleanupStack::PopAndDestroy(pContact);
            }
        else
            {
            PutNumber(aEvent, r);
            PutTime(aEvent, r);
            }

        }
}


namespace
{
    void GenerateHeading(TInt aIndex, request_rec* r)
        {
        for (TInt i = 0; i < KSizeEntries; i++)
            {
            if (i != aIndex)
                {
                ap_rputs(entries[i].pLink, r);
                ap_rputs("\n", r);
                }
            }
        }


    int GenerateLogL(CLogViewRecent& aLogView, 
                     TInt            aIndex, 
                     request_rec*    r)
        {
        GenerateHeading(aIndex, r);

        ap_rputs("<hr>\n", r);
        ap_rputs("<h3>", r);
        ap_rputs(entries[aIndex].pHeading, r);
        ap_rputs("</h3>", r);

        TPtrC8
            des(reinterpret_cast<const TUint8*>(entries[aIndex].pDefault));
            
        CLogEventGenerator
            *pGenerator = CLogEventGenerator::NewLC(&aLogView, 
                                                    entries[aIndex].idList,
                                                    &des,
                                                    r);
        
        pGenerator->GenerateL();
        
        CleanupStack::PopAndDestroy(pGenerator);

        GenerateHeading(aIndex, r);
        
        return OK;
        }


    int GenerateLogL(TInt aIndex, request_rec* r)
        {
        int
            rc = OK;
        RFs
            fs;
        
        User::LeaveIfError(fs.Connect());
        
        CleanupClosePushL(fs);
        
        CLogClient
            *pLogClient = CLogClient::NewL(fs);
        
        CleanupStack::PushL(pLogClient);
        
        CLogViewRecent
            *pLogView = CLogViewRecent::NewL(*pLogClient);

        CleanupStack::PushL(pLogView);

        rc = GenerateLogL(*pLogView, aIndex, r);
        
        CleanupStack::PopAndDestroy(pLogView); // pLogClient
        CleanupStack::PopAndDestroy(pLogClient); // pLogClient
        CleanupStack::PopAndDestroy(); // fs
        
        return rc;
        }


    int GenerateLogL(request_rec* r)
        {
        int
            rc = HTTP_NOT_FOUND;
        
        if (strcmp(r->args, "incoming") == 0)
            rc = GenerateLogL(KIndexIncoming, r);
        else if (strcmp(r->args, "outgoing") == 0)
            rc = GenerateLogL(KIndexOutgoing, r);
        else if (strcmp(r->args, "missed") == 0)
            rc = GenerateLogL(KIndexMissed, r);
        
        return rc;
        }


}


namespace
{
    int GenerateLogList(request_rec* r)
        {
        ap_rputs("<h3>Recent Calls</h3>\n", r);
        ap_rputs("<ul>\n", r);
        ap_rputs("  <li> <a href=\"logs?missed\">Missed calls</a></li>", r);
        ap_rputs("  <li> <a href=\"logs?incoming\">Received calls</a></li>", 
                 r);
        ap_rputs("  <li> <a href=\"logs?outgoing\">Dialled numbers</a></li>", 
                 r);
        ap_rputs("</ul>\n", r);
        
        return OK;
        }
}


namespace
{
    int GenerateContentL(request_rec* r)
        {
        int
            rc;
        
        if (r->args)
            rc = GenerateLogL(r);
        else
            rc = GenerateLogList(r);
        
        return rc;
        }
    
    
    int LogsHandler(request_rec* r)
        {
        __UHEAP_MARK;

        int
            rc = HTTP_INTERNAL_SERVER_ERROR;
            
        TRAPD(error, rc = GenerateContentL(r));

        __UHEAP_MARKEND;
        
        return rc;
        }
}


namespace
{
    int logs_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "logs") == 0) {
            ap_set_content_type(r, "text/html");
            ap_rputs(DOCTYPE_HTML_3_2
                     "<html><head>\n"
                     "<title>My Logs</title>\n"
                     "</head>\n"
                     "<body>\n", r);
            
            rc = LogsHandler(r);
            
            ap_rputs("</body>", r);
            ap_rputs("</html>", r);
        }
        
        return rc;
    }


    void register_hooks(apr_pool_t* p) {
        ap_hook_handler(logs_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }
}


extern "C"
{

module AP_MODULE_DECLARE_DATA symbian_logs_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,                       /* dir config creater */
    NULL,                       /* dir merger --- default is to override */
    NULL,                       /* server config */
    NULL,                       /* merge server config */
    NULL,                       /* command table */
    register_hooks              /* register_hooks */
};

}
