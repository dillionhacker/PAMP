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


/* Dos Module - Denial of Service
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /dos>
 * SetHandler dos
 * </Location>
 *
 * You may want to protect this location by password or domain so no one
 * else can look at it.  Then you can cause a dos attach with an URL like:
 *
 * http://your_server_name/dos
 *
 * /dos?n&m - Returns n bytes written using m byte chunks.
 *
 */


namespace
{
    void GenerateData(char* pBegin, const char* pEnd)
        {
        char
            *i = pBegin;
        int
            lineOffset   = 0,
            columnOffset = 0;
        
        while (i != pEnd)
            {
            if (columnOffset == 99)
                {
                *i = '\n';
                
                columnOffset = 0;
                ++lineOffset;
                }
            else
                {
                int
                    offset = (columnOffset + lineOffset + 1) % 10;
                
                *i = '0' + offset;
                
                ++columnOffset;
                }
            
            ++i;
            }
        }


    int GenerateContent(char*        pBegin,
                        const char*  pEnd,
                        TInt         chunk, 
                        request_rec* r)
        {
        int
            rc;
        
        GenerateData(pBegin, pEnd);
        
        char
            *i = pBegin;
        TInt
            nWritten = 0;
        
        do
            {
            TInt
                nWrite = Min(chunk, pEnd - i);
            
            nWritten = ap_rwrite(i, nWrite, r);
            
            if (nWritten > 0)
                i += nWritten;
            }
        while ((i != pEnd) && (nWritten > 0));
        
        if (nWritten >= 0)
            rc = OK;
        else
            rc = HTTP_INTERNAL_SERVER_ERROR;

        return rc;
        }


    int GenerateContent(TInt total, TInt chunk, request_rec* r)
        {
        int
            rc;
        
        if (total >= 0 && chunk > 0)
            {
            
            char
                *pBytes = (char*) malloc(total);
            
            if (pBytes)
                {
                rc = GenerateContent(pBytes, pBytes + total,
                                     chunk,
                                     r);

                free(pBytes);
                }
            else
                rc = HTTP_INTERNAL_SERVER_ERROR;
            }
        else
            rc = HTTP_BAD_REQUEST;

        return rc;
        }


    int GenerateContent(const char*  pTotal, 
                        const char*  pChunk,
                        request_rec* r)
        {
        int
            rc;
        TLex8
            lexer;
        
        lexer.Assign(reinterpret_cast<const TUint8*>(pTotal));
        
        TInt
            total;
        
        rc = lexer.Val(total);
        
        if (rc == KErrNone)
            {
            lexer.Assign(reinterpret_cast<const TUint8*>(pChunk));
            
            TInt
                chunk;
            
            rc = lexer.Val(chunk);

            if (rc == KErrNone)
                rc = GenerateContent(total, chunk, r);
            else
                rc = HTTP_BAD_REQUEST;
            }
        else
            rc = HTTP_BAD_REQUEST;

        return rc;
        }


    int GenerateContent(char* pArgs, request_rec* r)
        {
        const char
            *pTotal = pArgs,
            *pChunk;
        
        while (*pArgs != 0 && *pArgs != '&')
            ++pArgs;
        
        if (*pArgs != 0)
            {
            *pArgs = 0;
            
            ++pArgs;
            
            pChunk = pArgs;
            }
        else
            pChunk = pTotal;
            
        return GenerateContent(pTotal, pChunk, r);
        }
}


namespace
{
    int GenerateContentL(request_rec* r)
        {
        int
            rc;
        
        if (r->args)
            rc = GenerateContent(r->args, r);
        else
            rc = OK;
        
        return rc;
        }
    
    
    int DosHandler(request_rec* r)
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
    int dos_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "dos") == 0) {
            ap_set_content_type(r, "text/plain");
        
            rc = DosHandler(r);
        }
        
        return rc;
    }


    void register_hooks(apr_pool_t* p) {
        ap_hook_handler(dos_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }
}


extern "C"
{

module AP_MODULE_DECLARE_DATA symbian_dos_module =
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
