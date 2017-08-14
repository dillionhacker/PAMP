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
#include <mtclreg.h>
#include <msvapi.h>
#include <msvids.h>
#include <smsclnt.h>
#include <smut.h>
// Should the inclusion of the following two files fail, you are most
// likely using S60v2.0 from which they have inadvertenty(?) been
// left out.
//
// From the Symbian web-site look for FAQ-0980 and follow the 
// instructions there. Last time I checked, the URL was:
//
// http://www3.symbian.com/faq.nsf/0/7B5E8CCBA8AEEF7780256E3900506203?OpenDocument
//
#include <smuthdr.h>
#include <smutset.h>

#include "symbian_mod_utils.h"

// Apache httpd
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"


/* Messages Module - displays the messages on a Symbian phone.
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /messages>
 * SetHandler messages
 * </Location>
 *
 * You may want to protect this location by password or domain so no one
 * else can look at it.  Then you can access the contacts an URL like:
 *
 * http://your_server_name/messages
 *
 * /messages   - Returns all messages
 *
 */


namespace
{
    typedef int (FolderHandler)(request_rec* r);
    
    const struct Folder
    {
    const char* name;
    TMsvId      id;
    } folders[] =
    {
    {
    "Inbox",
    KMsvGlobalInBoxIndexEntryId
    },
    {
    "Outbox",
    KMsvGlobalOutBoxIndexEntryId    
    },
    {
    "Drafts",
    KMsvDraftEntryId
    },
    {
    "Sent",
    KMsvSentEntryId
    }
    };


    const TInt nFolders = sizeof(folders)/sizeof(folders[0]);
}


namespace
{
    class CMsvSessionObserver : public CBase,
                                public MMsvSessionObserver
        {
    public:
        static CMsvSessionObserver* NewLC();
        
    public:
        void HandleSessionEvent(TMsvSessionEvent aEvent,
                                TAny*            apArg1,
                                TAny*            apArg2,
                                TAny*            apArg3)
            {
            }
        
        
        void HandleSessionEventL(TMsvSessionEvent aEvent,
                                TAny*            apArg1,
                                TAny*            apArg2,
                                TAny*            apArg3)
            {
            }

    private:
        CMsvSessionObserver();
        };


    CMsvSessionObserver::CMsvSessionObserver()
        {
        }


    CMsvSessionObserver* CMsvSessionObserver::NewLC()
        {
        CMsvSessionObserver
            *pThis = new (ELeave) CMsvSessionObserver;

        CleanupStack::PushL(pThis);

        return pThis;
        }
}

namespace
{
    void GenerateContentL(CSmsHeader& header, request_rec* r)
        {
        const CArrayPtrFlat<CSmsNumber>
            &recipients = header.Recipients();
        
        ap_rputs("<b>Recipients: </b>", r);
        
        for (TInt i = 0; i < recipients.Count(); ++i)
            {
            const CSmsNumber
                &recipient = *recipients.At(i);
            TPtrC
                name = recipient.Name();
            
            if (name.Length() != 0)
                ap_rputdL(name, r);
            else
                ap_rputdL(recipient.Address(), r);
            
            if (i < recipients.Count() - 1)
                ap_rputs(", ", r);
            }
        
        ap_rputs("\n", r);
        ap_rputs("<br>", r);
        
        CSmsMessage
            &message = header.Message();
        
        CSmsBufferBase
            &buffer = message.Buffer();

        HBufC
            *pBody = HBufC::NewLC(buffer.Length());
        TPtr16
            body = pBody->Des();
        
        buffer.Extract(body, 0, buffer.Length());
        
        ap_rputs("<b>Body: </b>", r);
        ap_rputdL(*pBody, r);
        ap_rputs("\n", r);

        CleanupStack::PopAndDestroy(pBody);
        
        ap_rputs("<hr>\n", r);
        }


    void GenerateContentL(TMsvId         id,
                          CSmsClientMtm* pMtm,
                          CMsvSession*   pSession, 
                          request_rec*   r)
        {
        CMsvEntry
            *pRoot = pSession->GetEntryL(id);
        
        CleanupStack::PushL(pRoot);

        TMsvSelectionOrdering
            ordering(KMsvNoGrouping, EMsvSortByNone, ETrue);
        
        pRoot->SetSortTypeL(ordering);

        TInt
            count   = pRoot->Count(),
            entries = 0;
        
        for (TInt i = 0; i < count; i++)
            {
            const TMsvEntry
                &entry = (*pRoot)[i];

            if (entry.iMtm == KUidMsgTypeSMS)
                {
                CMsvEntry
                    *pEntry = pSession->GetEntryL(entry.Id());
                
                pMtm->SetCurrentEntryL(pEntry);
                pMtm->LoadMessageL();
                
                GenerateContentL(pMtm->SmsHeader(), r);

                ++entries;
                }
            else
                {
                // FIXME: Deal with other messages as well!
                }
            }

        if (entries == 0)
            ap_rputs("<h4>(No Messages)</h4>", r);

        CleanupStack::PopAndDestroy(pRoot);
        }


    int GenerateContentL(TMsvId id, request_rec* r)
        {
        CMsvSessionObserver
            *pObserver = CMsvSessionObserver::NewLC();
        CMsvSession
            *pSession = CMsvSession::OpenSyncL(*pObserver);
        
        CleanupStack::PushL(pSession);
        
        CClientMtmRegistry
            *pRegistry = CClientMtmRegistry::NewL(*pSession);
        
        CleanupStack::PushL(pRegistry);
        
        CBaseMtm
            *pBaseMtm = pRegistry->NewMtmL(KUidMsgTypeSMS);
        CSmsClientMtm
            *pMtm = static_cast<CSmsClientMtm*>(pBaseMtm);
        
        CleanupStack::PushL(pMtm);
        
        GenerateContentL(id, pMtm, pSession, r);
        
        CleanupStack::PopAndDestroy(pMtm);
        CleanupStack::PopAndDestroy(pRegistry);
        CleanupStack::PopAndDestroy(pSession);
        CleanupStack::PopAndDestroy(pObserver);

        return OK;
        }
    

    int GenerateTOC(request_rec* r)
        {
        ap_rputs("<h4>Messaging</h4>\n", r);
        
        ap_rputs("<ul>\n", r);
        
        for (int i = 0; i < nFolders; i++)
            {
            const Folder
                &folder = folders[i];
            
            ap_rputs("  <li> <a href=\"messages?", r);
            ap_rputs((const char*) folder.name, r);
            ap_rputs("\">", r);
            
            ap_rputs((const char*) folder.name, r);
            
            ap_rputs("</li>\n", r);
            }
        

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
        
        if (!r->args)
            rc = GenerateTOC(r);
        else
            {
            int
                i;
            
            for (i = 0; i < nFolders; i++)
                {
                if (strcmp(folders[i].name, r->args) == 0)
                    break;
                }
            
            if (i < nFolders)
                rc = GenerateContentL(folders[i].id, r);
            else
                rc = HTTP_NOT_FOUND;
            }
        
        return rc;
        }


    int MessagesHandler(request_rec* r)
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
    int messages_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "messages") == 0) {
            ap_set_content_type(r, "text/html");
            ap_rputs(DOCTYPE_HTML_3_2
                     "<html><head>\n"
                     "<title>My Messages</title>\n"
                     "</head>\n"
                     "<body>\n", r);
            
            rc = MessagesHandler(r);
            
            ap_rputs("</body>", r);
            ap_rputs("</html>", r);
        }
        
        return rc;
    }


    void register_hooks(apr_pool_t* p) {
        ap_hook_handler(messages_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }
}


extern "C"
{

module AP_MODULE_DECLARE_DATA symbian_messages_module =
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
