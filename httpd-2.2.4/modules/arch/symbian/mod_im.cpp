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

#include "s60_mod_utils.h"

// Apache httpd
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"


/* im Module - display instant message on a Symbian phone.
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /im>
 * SetHandler im
 * </Location>
 *
 * You may want to protect this location by password or domain so no one
 * else can look at it.  Then you can access the contacts an URL like:
 *
 * http://your_server_name/im
 *
 * /im   - Shows instant message.
 *
 */


namespace
{
    _LIT(KThreadName, "Apache IM Thread");

    const TInt KMaxHeapSize = 0x10000;
    const TInt KGranularity = 8;
}


namespace
{
    using namespace Apache;


    class CDisplayer
        {
    public:
        ~CDisplayer()
            {
            iThread.Close();
            iSemaphore.Close();
            iMutex.Close();
            delete ipMessages;
            delete ipMessenger;
            }


        static CDisplayer* New()
            {
            CDisplayer
                *pThis;
            
            TRAPD(error, pThis = NewL());
            
            if (error != KErrNone)
                pThis = 0;

            return pThis;
            }


        static CDisplayer* NewL()
            {
            CDisplayer
                *pThis = NewLC();
            
            CleanupStack::Pop(pThis);
            
            return pThis;
            }


        static CDisplayer* NewLC()
            {
            CDisplayer
                *pThis = new (ELeave) CDisplayer;
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();
            
            return pThis;
            }
        
        
        void DisplayL(const char* pText8)
            {
            TPtrC8
                text8(reinterpret_cast<const TUint8*>(pText8));
            HBufC
                *pText = HBufC::NewLC(text8.Length());
        
            pText->Des().Copy(text8);
            
            iMutex.Wait();
            TRAPD(error, ipMessages->AppendL(pText));
            iMutex.Signal();

            if (error == KErrNone)
                {
                CleanupStack::Pop(pText);
                
                iSemaphore.Signal();
                }
            else
                {
                CleanupStack::PopAndDestroy(pText);

                User::Leave(error);
                }
            }

    private:
        CDisplayer()
            {
            }


        void ConstructL()
            {
            ipMessages = new (ELeave) CArrayPtrSeg<HBufC>(KGranularity);
            User::LeaveIfError(iMutex.CreateLocal());
            User::LeaveIfError(iSemaphore.CreateLocal(0));

            RHeap
                *pHeap = 0; // Current Heap.
#if !defined(EKA2)
            pHeap = RThread().Heap();
#endif

            User::LeaveIfError(iThread.Create(KThreadName,
                                              &CDisplayer::CallThread,
                                              0x5000, // Stacksize
                                              pHeap,
                                              this));

            iThread.Resume();
            }

    private:
        HBufC* Get()
            {
            iSemaphore.Wait();
            iMutex.Wait();
            
            HBufC
                *pText = ipMessages->At(0);
            
            ipMessages->Delete(0);

            iMutex.Signal();

            return pText;
            }

    private:
        static TInt CallThread(TAny* apThis)
            {
            return static_cast<CDisplayer*>(apThis)->Thread();
            }
        

        TInt Thread()
            {
            CTrapCleanup
                *pCleanup = CTrapCleanup::New();
            CActiveScheduler
                *pCurrent   = CActiveScheduler::Current(), // Should be 0.
                *pScheduler = 0;
        
            if (!pCurrent)
                {
                pScheduler = new (ELeave) CActiveScheduler;
                
                CActiveScheduler::Install(pScheduler);
                }

            TRAPD(error, ThreadL());

            if (pScheduler)
                {
                CActiveScheduler::Install(0);
                delete pScheduler;
                }
            
            delete pCleanup;

            return error;
            }
                               

        void ThreadL()
            {
            ipMessenger = CMessenger::NewL();

            while (HBufC* pText = Get())
                {
                DeliverMessage(*pText);

                delete pText;
                }
            }

        
        void DeliverMessage(const TDesC& aMessage)
            {
            TRAPD(error, DeliverMessageL(aMessage));
            }

        
        void DeliverMessageL(const TDesC& aMessage)
            {
            const TDesC
                &header = _L("Instant Message");
            TInt
                buttons = R_AVKON_SOFTKEYS_SAVE_BACK;
            
            switch (ipMessenger->DeliverL(header, aMessage, buttons))
                {
                case EAknSoftkeySave:
                    AddSMSToInboxL(header, aMessage, aMessage, false, false);
                    break;
                    
                case EAknSoftkeyBack:
                case CMessage::KTimedOut:
                default:
                    break;
                }
            }
        
    private:
        RMutex               iMutex;
        RSemaphore           iSemaphore;
        RThread              iThread;
        CArrayPtrSeg<HBufC>* ipMessages;
        CMessenger*          ipMessenger;
        };
}


namespace
{
    using namespace Apache;

    // FIXME: Potential race with multiple threads.

    
    CDisplayer* GetDisplayerL()
        {
        static CDisplayer* 
            pDisplayer;

        if (!pDisplayer)
            pDisplayer = CDisplayer::NewL();

        return pDisplayer;
        }


    CDisplayer* GetDisplayer()
        {
        CDisplayer
            *pDisplayer = 0;
        
        TRAPD(error, pDisplayer = GetDisplayerL());
        
        return pDisplayer;
        }
}


namespace
{
    using namespace Apache;


    int InboxHandlerL(CDisplayer& aDisplayer, request_rec* r)
        {
        QueryArgument
            *pArguments;
        int
            nArguments;
        int
            rc = ParseQueryString(r, &pArguments, &nArguments);

        if (rc == OK)
            {
            rc = HTTP_BAD_REQUEST;
            
            const char
                *pText;
            
            if (GetValue(pArguments, nArguments, "text", &pText))
                {
                URLDecode(pText, strlen(pText), const_cast<char*>(pText));
                    
                aDisplayer.DisplayL(pText);
                
                ap_rputs("    <p>\n"
                         "      Message delivered!\n"
                         "    </p>\n", r);
                
                rc = OK;
                }
            }

        return rc;
        }


    int InboxHandler(request_rec* r)
        {
        int
            rc = HTTP_INTERNAL_SERVER_ERROR;
        CDisplayer
            *pDisplayer = GetDisplayer();

        if (pDisplayer)
            {
            TRAPD(error, rc = InboxHandlerL(*pDisplayer, r));
            }

        return rc;
        }
}


namespace
{
    int inbox_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "im") == 0) {
            ap_set_content_type(r, "text/html");
            ap_rputs(DOCTYPE_HTML_3_2
                     "<html>\n"
                     "  <head>\n"
                     "    <title>Message Sent</title>\n"
                     "  </head>\n"
                     "  <body>\n", r);
            
            rc = InboxHandler(r);
            
            ap_rputs("  </body>\n", r);
            ap_rputs("</html>\n", r);
        }
        
        return rc;
    }


    void register_hooks(apr_pool_t* p) {
        ap_hook_handler(inbox_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }
}


extern "C"
{

module AP_MODULE_DECLARE_DATA s60_im_module =
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
