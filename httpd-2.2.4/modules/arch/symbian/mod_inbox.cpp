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

#include "symbian_mod_utils.h"

// Apache httpd
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"


/* Inbox Module - put messages in the inbox of a Symbian phone.
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /inbox>
 * SetHandler inbox
 * </inbox>
 *
 * You may want to protect this location by password or domain so no one
 * else can look at it.  Then you can access the contacts an URL like:
 *
 * http://your_server_name/inbox
 *
 * /inbox   - Accepts a message to be put in the inbox.
 *
 */


namespace
{
    using namespace Apache;
    
    
    int GenerateContentL(const char* pText, request_rec* r)
        {
        HBufC
            *pBuffer = str2HBufCLC(pText);
        
        AddSMSToInboxL(_L("The Web!"), *pBuffer, *pBuffer, true, true);
        
        CleanupStack::PopAndDestroy(pBuffer);

        return OK;
        }
}


namespace
{
    using namespace Apache;


    int InboxHandlerL(request_rec* r)
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
                    
                rc = GenerateContentL(pText, r);
                
                if (rc == OK)
                    ap_rputs("    <p>\n"
                             "      Message sent!\n"
                             "    </p>\n", r);
                }
            }

        return rc;
        }


    int InboxHandler(request_rec* r)
        {
        __UHEAP_MARK;
        
        int
            rc = HTTP_INTERNAL_SERVER_ERROR;
        
        TRAPD(error, rc = InboxHandlerL(r));
        
        __UHEAP_MARKEND;

        return rc;
        }
}


namespace
{
    int inbox_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "inbox") == 0) {
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

module AP_MODULE_DECLARE_DATA symbian_inbox_module =
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
