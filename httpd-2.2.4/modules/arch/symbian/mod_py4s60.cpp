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
#include <cspyinterpreter.h>

// Libc
#include <ctype.h>
#include <string.h>

// Symbian Apache
#include "symbian_mod_utils.h"

// APR
#include "apr_strings.h"

// Apache httpd
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "util_script.h"


/* Python Module - allows the execution of Python scripts.
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /python>
 * SetHandler python
 * </python>
 *
 * You may want to protect this location by password or domain so no one
 * else can look at it. Then you can access the what an URL like: 
 *
 *     http://your_server_name/python/script.py
 *
 * which causes the python script C:/Apache/python/script.py to be
 * executed.
 *
 * NOTE: The algorithm is as follows:
 *
 *       1) In the URL, everything after http://.../python/ is
 *          assumed to be a path-component.
 *          => "script.py".
 *       2) Then that path-component is appended to the
 *          hard-coded path "C:/Apache/python/".
 *          => "C:/Apache/python/script.py"
 *       3) That path is then assumed to refer to a python script.
 *
 * Script input:
 *   * Any CGI arguments are provided as one command-line argument
 *     to the script.
 *   * A possible request body is provided via stdin.
 *
 * Script output:
 *   * Anything up to the first '\n\n' sequence is assumed to be HTTP reply
 *     headers and treated as such. 
 *   * Anything after the first '\n\n' sequence is assumed to be the HTTP
 *     reply body.
 *
 * Thus, at minimum, the script MUST contain the line:
 *
 *   print "Content-type: text/html\n"
 *
 */


namespace
{
    _LIT8(KPythonScriptDir, "C:\\Apache\\Python\\");
}


namespace
{
    /*
     *
     */
    
    class CStdio : public CBase
        {
    public:
        ~CStdio()
            {
            free(ipLine);
            }


        static CStdio* NewLC(request_rec* r, TBool aBody)
            {
            CStdio
                *pStdio = new (ELeave) CStdio(r, aBody);
            
            CleanupStack::PushL(pStdio);

            return pStdio;
            }

        
        int Input(char* apBuffer, int anChars)
            {
            int
                rc;
            
            if (iBody)
                rc = ap_get_client_block(iR, apBuffer, anChars);
            else
                rc = -1;

            return rc;
            }

        
        int Output(const char* apBuffer, int anChars)
            {
            int
                rc;
            
            if (inNewlines == 2)
                // All HTTP reply header-lines have been read, so this is
                // body data.
                rc = ap_rwrite(apBuffer, anChars, iR);
            else 
                {
                rc = EnsureSpaceFor(anChars);
                
                if (rc == 0)
                    {
                    Analyze(apBuffer, anChars);

                    rc = anChars;
                    }
                }
            
            return rc;
            }

        
        int Errput(const char* apBuffer, int anChars)
            {
            return Output(apBuffer, anChars);
            }

    private:
        CStdio(request_rec* r, TBool aBody)
            : iR(r),
              iBody(aBody)
            {
            }

    private:
        enum 
            {
            DEFAULT_SIZE = 64
            };
        

        int Realloc(int aSize)
            {
            int
                rc = 0;
            
            if (inSize == 0)
                inSize = DEFAULT_SIZE;

            while (inSize < aSize)
                inSize *= 2;
                        
            char
                *pTmp = (char*) malloc(inSize);
            
            if (pTmp)
                {
                memcpy(pTmp, ipLine, inLength);
                
                free(ipLine);
                
                ipLine = pTmp;
                }
            else
                rc = -1;
            
            return rc;
            }


        int EnsureSpaceFor(int anChars)
            {
            int
                rc = 0;
            
            if ((inLength + anChars) >= inSize)
                rc = Realloc(inLength + anChars);

            return rc;
            }


        void Analyze(const char* apBuffer, int anChars)
            {
            const char
                *i   = apBuffer,
                *end = i + anChars;

            while (i != end)
                {
                switch (*i)
                    {
                    case '\n':
                        {
                        // We have got a full line "...\n".
                        inNewlines++;
                        
                        const char
                            *tail = i + 1;
                        int
                            nTail = end - tail;
                     
                        if (inNewlines == 2)
                            {
                            // Two consequtive newlines seen - the entire 
                            // header has been read. Remaining stuff belongs
                            // to the body.
                            
                            if (nTail != 0)
                                // So, we loose this return value.
                                ap_rwrite(tail, nTail, iR);
                            }
                        else
                            {
                            ipLine[inLength] = 0;
                            
                            AnalyzeLine();
                            
                            inLength = 0;
                            
                            if (nTail)
                                Analyze(tail, nTail);
                            }

                        i = end - 1; // To break out of loop.
                        }
                        break;

                    default:
                        inNewlines = 0;
                        
                        ipLine[inLength++] = *i;
                    }

                ++i;
                }
            }


        static inline char* RemovePrefixWS(char* pBegin, char* pEnd)
            {
            while ((pBegin != pEnd) && isspace(*pBegin))
                pBegin++;
            
            return pBegin;
            }


        static inline char* RemoveSuffixWS(char* pBegin, char* pEnd)
            {
            while ((pEnd > pBegin) && isspace(*(pEnd - 1)))
                {
                --pEnd;
                
                *pEnd = 0;
                }

            return pEnd;
            }
        

        void AnalyzeLine()
            {
            char
                *pKey = ipLine,
                *pEnd = ipLine + inLength;

            pEnd = RemoveSuffixWS(pKey, pEnd);
            pKey = RemovePrefixWS(pKey, pEnd);
            
            char
                *pSeparator = pKey;

            // Find ':' that separates key and value.
            while ((pSeparator != pEnd) && (*pSeparator != ':'))
                ++pSeparator;

            char
                *pValue = pSeparator;
            
            if (pValue != pEnd)
                {
                pValue = pSeparator + 1;
                
                pValue = RemovePrefixWS(pValue, pEnd);

                *pSeparator = 0;

                RemoveSuffixWS(pKey, pSeparator);
                }
            else
                pValue = "";

            SetValue(pKey, pValue);
            }


        void SetValue(const char* pKey, const char* pValue)
            {
            if (strcmp(pKey, "Content-type") == 0)
                ap_set_content_type(iR, apr_pstrdup(iR->pool, pValue));
            else
                apr_table_set(iR->headers_out, pKey, pValue);
            }

        
    private:
        request_rec* iR;
        TBool        iBody;      // Does the request has a body?
        int          inNewlines; // How many new-lines have been seen.
        char*        ipLine;     // A line is collected here before analyzed.
        int          inLength;   // Where in ipLine we are right now.
        int          inSize;     // Total size of ipLine.
        };


    /*
     * These functions are called by the Python interpreter in order to get
     * stdin data and to output stdout and stderr data.
     *
     * 'aCookie' is set by CStdioSettings below.
     */
     
    int Input(void* aCookie, char* apBuffer, int anChars)
        {
        CStdio
            *pStdio = static_cast<CStdio*>(aCookie);

        return pStdio->Input(apBuffer, anChars);
        }

    
    int Output(void* aCookie, const char* apBuffer, int nChars)
        {
        CStdio
            *pStdio = static_cast<CStdio*>(aCookie);
        
        return pStdio->Output(apBuffer, nChars);
        }
    

    int Errput(void* aCookie, const char* apBuffer, int nChars)
        {
        CStdio
            *pStdio = static_cast<CStdio*>(aCookie);
        
        return pStdio->Errput(apBuffer, nChars);
        }


    /*
     * CStdioSettings is a RAII-class whose constructor stores the current
     * stdio settings of this thread's _REENT structure and replaces them
     * with the Input, Output and Errput above.
     *
     * This is done so that we subsequently can provide the Python
     * interpreter - that is, the script it is running - with the body of
     * the HTTP request as stdin data and be provided with the script's
     * stdout and stderr output, which will be returned as the HTTP reply's
     * body.
     *
     * The destructor restores the situation.
     */

    class CStdioSettings : public CBase
        {
    public:
        static CStdioSettings* NewLC(CStdio* apStdio)
            {
            CStdioSettings
                *pSettings = new (ELeave) CStdioSettings(apStdio);
            
            CleanupStack::PushL(pSettings);
            
            return pSettings;
            }

        
        ~CStdioSettings()
            {
            RestoreSettings();
            }
        
    private:
        CStdioSettings(CStdio* apStdio)
            {
            SaveSettings();
            SetSettings(apStdio);
            }

        
        void SaveSettings()
            {
            struct __sFILE
                *pFile;
            
            pFile = &_REENT->_sf[0];
            
            iICookie = pFile->_cookie;
            iIRead   = pFile->_read;
            iIWrite  = pFile->_write;
            iISeek   = pFile->_seek;
            iIClose  = pFile->_close;

            pFile = &_REENT->_sf[1];
            
            iOCookie = pFile->_cookie;
            iORead   = pFile->_read;
            iOWrite  = pFile->_write;
            iOSeek   = pFile->_seek;
            iOClose  = pFile->_close;

            pFile = &_REENT->_sf[2];
            
            iECookie = pFile->_cookie;
            iERead   = pFile->_read;
            iEWrite  = pFile->_write;
            iESeek   = pFile->_seek;
            iEClose  = pFile->_close;

            iSdidinit = _REENT->__sdidinit;
            }
        
        
        void RestoreSettings()
            {
            struct __sFILE
                *pFile;
            
            pFile = &_REENT->_sf[0];
            
            pFile->_cookie = iICookie;
            pFile->_read   = iIRead;
            pFile->_write  = iIWrite;
            pFile->_seek   = iISeek;
            pFile->_close  = iIClose;

            pFile = &_REENT->_sf[1];
            
            pFile->_cookie = iOCookie;
            pFile->_read   = iORead;
            pFile->_write  = iOWrite;
            pFile->_seek   = iOSeek;
            pFile->_close  = iOClose;
            
            pFile = &_REENT->_sf[2];
            
            pFile->_cookie = iECookie;
            pFile->_read   = iERead;
            pFile->_write  = iEWrite;
            pFile->_seek   = iESeek;
            pFile->_close  = iEClose;
            
            _REENT->__sdidinit = iSdidinit;
            }


        void SetSettings(CStdio* apStdio)
            {
            struct __sFILE
                *pFile;
            
            pFile = &_REENT->_sf[0];
            
            pFile->_cookie = apStdio;
            pFile->_read   = Input;
            pFile->_write  = 0;
            pFile->_seek   = 0;
            pFile->_close  = 0;
            
            pFile = &_REENT->_sf[1];
            
            pFile->_cookie = apStdio;
            pFile->_read   = 0;
            pFile->_write  = Output;
            pFile->_seek   = 0;
            pFile->_close  = 0;
            
            pFile = &_REENT->_sf[2];
            
            pFile->_cookie = apStdio;
            pFile->_read   = 0;
            pFile->_write  = Errput;
            pFile->_seek   = 0;
            pFile->_close  = 0;
            
            _REENT->__sdidinit = ETrue;
            }


    private:
        void*     iICookie;
        int		(*iIRead) (void * _cookie, char *_buf, int _n);
        int		(*iIWrite)(void * _cookie, const char *_buf, int _n);
        _fpos_t	(*iISeek) (void * _cookie, _fpos_t _offset, int _whence);
        int		(*iIClose)(void * _cookie);
        void*     iOCookie;
        int		(*iORead) (void * _cookie, char *_buf, int _n);
        int		(*iOWrite)(void * _cookie, const char *_buf, int _n);
        _fpos_t	(*iOSeek) (void * _cookie, _fpos_t _offset, int _whence);
        int		(*iOClose)(void * _cookie);
        void*     iECookie;
        int		(*iERead) (void * _cookie, char *_buf, int _n);
        int		(*iEWrite)(void * _cookie, const char *_buf, int _n);
        _fpos_t	(*iESeek) (void * _cookie, _fpos_t _offset, int _whence);
        int		(*iEClose)(void * _cookie);
        TBool     iSdidinit;
        };
    }


namespace
{
    void InitStdio(void*)
        {
        /* Everything is done via CStdioSettings, but this needs to be here to
           prevent the default initialization done by CSPyInterpreter.
        */

        _REENT->__sdidinit = ETrue;
        }
}


namespace
{
    int GenerateContentL(CSPyInterpreter& aInterpreter, request_rec* r)
        {
        int
            rc = OK;

        if (r->path_info)
            {
            TBuf8<KMaxFileName + 1>
                path;
            
            path.Copy(KPythonScriptDir());
            
            TPtrC8 // +1 to remove initial '/'
                file(reinterpret_cast<TUint8*>(r->path_info + 1)); 
            
            if (path.Length() + file.Length() <= KMaxFileName)
                {
                path.Append(file);

                char
                    *argv[3];
                const char
                    *script = reinterpret_cast<const char*>(path.PtrZ());

                argv[0] = const_cast<char*>(script);
                argv[1] = r->args;
                argv[2] = 0;

                int
                    argc = r->args ? 2 : 1;

                TInt
                    error = aInterpreter.RunScript(argc, argv);

                switch (error)
                    {
                    default:
                        /*
                         * rc = HTTP_INTERNAL_SERVER_ERROR;
                         * break;
                         *
                         * For the time being we let it through so as
                         * not to swallow the output. Easier to debug
                         * that way.
                         */
                        
                    case KErrNone:
                        rc = OK;
                        break;

                    case KErrNotFound:
                        rc = HTTP_NOT_FOUND;
                        break;
                    }
                }
            else
                rc = HTTP_REQUEST_ENTITY_TOO_LARGE;
            }

        return rc;
        }
}


namespace
{
    int GenerateContentL(request_rec* r)
        {
        // FIXME: Has to be changed when threads are introduced.
        // FIXME: Should be deleted at some point.
        
        static CSPyInterpreter
            *pInterpreter;

        if (!pInterpreter)
            {
            int
                sdidinit = _REENT->__sdidinit;

            pInterpreter = CSPyInterpreter::NewInterpreterL(EFalse, InitStdio);

            _REENT->__sdidinit = sdidinit;
            }
        
        CStdio
            *pStdio = CStdio::NewLC(r, ap_should_client_block(r));
        CStdioSettings
            *pSettings = CStdioSettings::NewLC(pStdio);
        
        int
            rc = GenerateContentL(*pInterpreter, r);

        CleanupStack::PopAndDestroy(pSettings);
        CleanupStack::PopAndDestroy(pStdio);

        return rc;
        }


    int PythonHandler(request_rec* r)
        {
        int
            rc = HTTP_INTERNAL_SERVER_ERROR;
        
        TRAPD(error, rc = GenerateContentL(r));
        
        return rc;
        }
}


namespace
{
    int python_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "python") == 0) {
            rc = ap_setup_client_block(r, REQUEST_CHUNKED_DECHUNK);
            
            if (rc == OK)
                rc = PythonHandler(r);
        }
         
        return rc;
    }


    void register_hooks(apr_pool_t* p) {
        ap_hook_handler(python_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }
}


extern "C"
{

module AP_MODULE_DECLARE_DATA symbian_py4s60_module =
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
