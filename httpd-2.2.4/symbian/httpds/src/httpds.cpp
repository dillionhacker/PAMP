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

#include "httpds/chttpdserver.h"
#include <flogger.h>


#if defined(_DEBUG)

/*
 * Carbide can on-target debug a DLL that an EXE explicitly depends 
 * on, but not a DLL that a DLL that an EXE explicitly depends on.
 *
 * Consequently we explictly force a dependency between httpds.exe
 * and libapr and libaprutil. libhttpd httpds.exe depends upon
 * anyway.
 * 
 */ 

#include <apr_version.h>
#include <apu_version.h>


void force_explicit_dependency_on_apr_and_aprutil()
    {
    apr_version(0);
    apu_version(0);
    }

#endif


namespace
{
    using namespace Mws;
    using namespace Httpd;
    
    
    void MainL()
        {
        CActiveScheduler
            *pScheduler = new (ELeave) CActiveScheduler;
        
        CleanupStack::PushL(pScheduler);
        
        CActiveScheduler::Install(pScheduler);
        
        ::CServer2
            *pServer = HttpdS::CHttpdServer::NewLC(CActive::EPriorityStandard);
        
        RSemaphore
            semaphore;
        TInt
            rc = semaphore.OpenGlobal(KServerSemaphoreName); 
        
        switch (rc)
            {
            case KErrNone:
                semaphore.Signal();
                semaphore.Close();
                break;
                
            case KErrNotFound:
                /*
                 * The semaphore is not found. Most likely this is caused
                 * by httpds.exe having been started directly and not via
                 * the client library. Hence we just ignore it.
                 */
                break;
                
            default:
                User::Leave(rc);
            }
        
        CActiveScheduler::Start();

        CleanupStack::PopAndDestroy(pServer);
        CleanupStack::PopAndDestroy(pScheduler);
        }


    TInt Main()
        {
        CTrapCleanup
            *pCleanupStack = CTrapCleanup::New();
        
        if (pCleanupStack == 0)
            HttpdS::CHttpdServer::PanicServer(EHttpdPanicServerCreate);
        
        TRAPD(err, MainL());
                 
        if (err != KErrNone)
            HttpdS::CHttpdServer::PanicServer(EHttpdPanicServerCreate);
        
        delete pCleanupStack;
        
        return KErrNone;
        }
}


namespace
{
    _LIT(KLogDir,  "mws");
    _LIT(KLogName, "httpds.log");
}
    
    
namespace HttpdS
{
    void PanicClient(const RMessage2& aMessage, TInt aPanic)
        {
        aMessage.Panic(KServerFileName, aPanic);
        }

    
    void WriteLog(const TDesC& aText)
        {
        RFileLogger::Write(KLogDir, KLogName, 
                           EFileLoggingModeAppend, aText);
        }
}


TInt E32Main()
    {
    return Main();
    }
