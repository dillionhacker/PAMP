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
#include <es_sock.h>
#include <bt_sock.h>

#include "symbian_mod_utils.h"

// Apache httpd
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"


/* Proximity Module - displays the Bluetooth devices in the proximity.
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /proximity>
 * SetHandler proximity
 * </Location>
 *
 * You may want to protect this location by password or domain so no one
 * else can look at it.  Then you can access the proximuty with an URL like:
 *
 * http://your_server_name/proximity
 *
 * /proximity   - Returns proximity.
 *
 */


namespace
{
    const TInt KSizeBTAddress = 12; // "000000000000";
}


namespace
{
    NONSHARABLE_CLASS(CProximityScanner) : public CActive
        {
    public:
        ~CProximityScanner();
        
        static CProximityScanner* NewLC(RHostResolver hResolver,
                                        request_rec*  r);

        
        void ScanL();
        
    private:
        void RunL();
        
        void DoCancel();
        
    private:
        CProximityScanner(RHostResolver hResolver,
                          request_rec*  r);

        void ConstructL();

        void DoRunL();
        
        void StartL();
        void IterateL();
        void Stop();

        enum TState
            {
            EIdle,
            EQuerying,
            EIterating
            };

        void SetState(TState aState);

    private:
        RHostResolver         ihResolver;
        request_rec*          r;
        TState                iState;
        CActiveSchedulerWait* ipScheduler;
        TInquirySockAddr      iAddress;
        TNameEntry            iName;
        };


    CProximityScanner::CProximityScanner(RHostResolver hResolver,
                                         request_rec*  rr)
        : CActive(CActive::EPriorityStandard),
          ihResolver(hResolver),
          r(rr),
          iState(EIdle)
        {
        CActiveScheduler::Add(this);
        }
        
    
    CProximityScanner::~CProximityScanner()
        {
        delete ipScheduler;
        }


    CProximityScanner* CProximityScanner::NewLC(RHostResolver hResolver,
                                                request_rec*  r)
        {
        CProximityScanner
            *pThis = new (ELeave) CProximityScanner(hResolver, r);
        
        CleanupStack::PushL(pThis);
        
        pThis->ConstructL();
        
        return pThis;
        }

    
    void CProximityScanner::ConstructL()
        {
        ipScheduler = new (ELeave) CActiveSchedulerWait;
        }


    void CProximityScanner::ScanL()
        {
        RunL();
        }


    void CProximityScanner::RunL()
        {
        TRAPD(error, DoRunL());
        
        if (error != KErrNone)
            Stop();
        }


    void CProximityScanner::DoCancel()
        {
        ihResolver.Cancel();
        }



    void CProximityScanner::DoRunL()
        {
        switch (iState)
            {
            case EIdle:
                StartL();
                break;
                
            case EQuerying:
                IterateL();
                break;
                
            case EIterating:
                IterateL();
                break;
            }
        }
    
    
    void CProximityScanner::StartL()
        {
        iAddress.SetIAC(KGIAC);
        iAddress.SetAction(KHostResInquiry);
            
        ihResolver.GetByAddress(iAddress, iName, iStatus);
 
        SetState(EQuerying);
        SetActive();
        ipScheduler->Start();
        }


    void CProximityScanner::IterateL()
        {
        
        if (iStatus.Int() == KErrNone)
            {
            TNameRecord
                record = iName();
            const TBTSockAddr
                &address = static_cast<TBTSockAddr&>(record.iAddr);
            TBTDevAddr
                btAddress = address.BTAddr();

            ap_rputs("<!--bda:", r);
            
            TBuf<KSizeBTAddress>
                readable;
            
            btAddress.GetReadable(readable);
            
            ap_rputdL(readable, r);
            ap_rputs("-->\n", r);
 
            ihResolver.Next(iName, iStatus);
            SetActive();
            }
        else
            Stop();
        }


    void CProximityScanner::Stop()
        {
        ipScheduler->AsyncStop();
        
        SetState(EIdle);
        }


    void CProximityScanner::SetState(TState aState)
        {
        iState = aState;
        }
}


namespace
{
#if defined(__WINS__)
    int GenerateContentL(request_rec* r)
        {
        ap_rputs("<h4>Nearby Mobile Web-sites</h4>\n", r);
        ap_rputs("<ul>\n", r);
        ap_rputs("<!--bda:00aa11bb22cc-->\n", r);
        ap_rputs("</ul>\n", r);
        
        return OK;
        }
#else
    int GenerateContentL(request_rec* r)
        {
        RSocketServ
            hServer;
        
        User::LeaveIfError(hServer.Connect());
        CleanupClosePushL(hServer);
        
        TProtocolDesc
            info;
        
        _LIT(KL2Cap, "BTLinkManager");
        
        User::LeaveIfError(hServer.FindProtocol(KL2Cap(), info));
        
        RHostResolver
            hResolver;
        TInt
            error = hResolver.Open(hServer, info.iAddrFamily, info.iProtocol);
        
        if (error == KErrNone)
            {
            CleanupClosePushL(hResolver);

            CProximityScanner
                *pScanner = CProximityScanner::NewLC(hResolver, r);
            
            ap_rputs("<h4>Nearby Mobile Web-sites</h4>\n", r);
            ap_rputs("<ul>\n", r);
        
            pScanner->ScanL();

            ap_rputs("</ul>\n", r);

            CleanupStack::PopAndDestroy(pScanner);
            CleanupStack::PopAndDestroy(); // hResolver
            }
        else
            ap_rputs("Could not open BT resolver.\n", r);

        CleanupStack::PopAndDestroy(); // hServer
        
        return OK;
        }
#endif


    int ProximityHandler(request_rec* r)
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
    int proximity_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "proximity") == 0) {
            ap_set_content_type(r, "text/html");
            apr_table_set(r->headers_out, "Ned-Cmd", "bda2url");
            
            ap_rputs(DOCTYPE_HTML_3_2
                     "<html><head>\n"
                     "<title>My Proximity</title>\n"
                     "</head>\n"
                     "<body>\n", r);
        
            rc = ProximityHandler(r);
            
            ap_rputs("</body>", r);
            ap_rputs("</html>", r);
        }
        
        return rc;
    }


    void register_hooks(apr_pool_t* p) {
        ap_hook_handler(proximity_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }
}


extern "C"
{

module AP_MODULE_DECLARE_DATA symbian_proximity_module =
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
