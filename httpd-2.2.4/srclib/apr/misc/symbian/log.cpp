/* Copyright 2007 Nokia Corporation
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

#include <flogger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apr.h"


namespace
{
    void AppendLogL(const TDesC&  aDir,
                    const TDesC&  aFile,
                    const TDesC8& aText)
        {
        RFileLogger::Write(aDir, aFile, EFileLoggingModeAppend, aText);
        }


    void AppendLogL(const TDesC8& aDir,
                    const TDesC8& aFile,
                    const TDesC8& aText)
        {
        HBufC
            *pDir  = HBufC::NewLC(aDir.Length()),
            *pFile = HBufC::NewLC(aFile.Length());

        pDir->Des().Copy(aDir);
        pFile->Des().Copy(aFile);

        AppendLogL(*pDir, *pFile, aText);
        
        CleanupStack::PopAndDestroy(pFile);
        CleanupStack::PopAndDestroy(pDir);
        }


    void AppendLogL(const char* apDir,
                    const char* apFile,
                    const char* pText, 
                    int         length)
        {
        TPtrC8
            dir(reinterpret_cast<const TUint8*>(apDir), strlen(apDir)),
            file(reinterpret_cast<const TUint8*>(apFile), strlen(apFile)),
            text(reinterpret_cast<const TUint8*>(pText), length);

        AppendLogL(dir, file, text);
        }
    

    void AppendLog(const char* apDir, 
                   const char* apFile,
                   const char* apFormat, 
                   va_list     args)
        {
        char
            *pText = 0;
        int
            length = vasprintf(&pText, apFormat, args);
        
        if (pText)
            {
            TRAPD(dummy, AppendLogL(apDir, apFile, pText, length));

            free(pText);
            }
        }
    }


extern "C"
{

APR_DECLARE(void) symbian_log_append(const char* apDir,
                                     const char* apFile,
                                     const char* apFormat, ...)
{
    va_list args;
    
    va_start(args, apFormat);
    AppendLog(apDir, apFile, apFormat, args);
    va_end(args);
}

} // extern "C"
