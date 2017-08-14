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

#include "mws/rhttpd.h"
#include "httpdc/private.h"


using namespace Mws;
using namespace Httpd;
using namespace HttpdC;


namespace
{
    enum
        {
        KAsyncMessageSlots = 1
        };
}


namespace
{
    TInt CreateServer()
        {
        RProcess
            server;
        TInt
            rc = server.Create(KServerFileName, KNullDesC);
        
        if (KErrNone == rc) 
            {
            server.Resume();
            server.Close();
            }
        else
            WriteLog(_L("Server not started."));
        
        return rc;
        }
}


namespace
{
    TInt DoStartServer()
        {
        TFindServer
            finder(KServerName);
        TFullName
            name;
        
        TInt
            rc = finder.Next(name);
        
        if (KErrNone != rc)
            {
            // Server not running
            
            RSemaphore
                semaphore;
            
            rc = semaphore.CreateGlobal(KServerSemaphoreName, 0);
            
            if (KErrNone == rc) 
                {
                rc = CreateServer();
                
                if (rc == KErrNone)
                    semaphore.Wait();
                
                semaphore.Close();
                }
            }
        
        return rc;
        }
}


namespace Mws
{
    EXPORT_C RHttpd::RHttpd()
        {
        }
    
    
    EXPORT_C TInt RHttpd::Connect()
        {
        TInt
            rc = DoStartServer();
        
        if (KErrNone == rc)
            rc = CreateSession(KServerName,
                               Version(),
                               KAsyncMessageSlots);
        else
            WriteLog(_L("Creating session failed."));

        return rc;
        }
    
    
    EXPORT_C void RHttpd::Close()
        {
        RSessionBase::Close();
        }
    
    
    EXPORT_C TVersion RHttpd::Version() const
        {
        return TVersion(KProtocolVersionMajor, KProtocolVersionMinor, 0);
        }
    
    
    EXPORT_C TInt RHttpd::StartServer(const THttpdSettings& aSettings) const
        {
        TPckgC<THttpdSettings>
            arg(aSettings);
        TIpcArgs
            args;

        args.Set(1, &arg);
        
        return SendReceive(EHttpdReqStart, args);
        }
    
    
    EXPORT_C TInt RHttpd::StartServer(const TDesC&          aCommandLine,
                                      const THttpdSettings& aSettings) const
        {
        TPckgC<THttpdSettings>
            arg1(aSettings);
        TIpcArgs
            args;

        args.Set(1, &arg1);
        args.Set(2, &aCommandLine);
        
        return SendReceive(EHttpdReqStart2, args);
        }
    
    
    EXPORT_C TInt RHttpd::StopServer(TBool aGraceful) const
        {
        TIpcArgs
            args;
        
        args.Set(1, static_cast<TInt>(aGraceful));
        
        return SendReceive(EHttpdReqStop, args);
        }

    
    EXPORT_C TInt RHttpd::RestartServer(TBool aGraceful) const
        {
        TIpcArgs
            args;
        
        args.Set(1, static_cast<TInt>(aGraceful));
        
        return SendReceive(EHttpdReqRestart, args);
        }
    

    EXPORT_C TInt RHttpd::GetState(THttpdState& aState) const
        {
        TPckg<THttpdState>
            arg(aState);
        TIpcArgs
            args;
        
        args.Set(0, &arg);
        
        return SendReceive(EHttpdReqGetState, args);
        }


    EXPORT_C void RHttpd::GetState(TPckg<THttpdState>* apState, 
                                   TRequestStatus&     aStatus) const
        {
        TIpcArgs
            args;
        
        args.Set(0, apState);
        
        SendReceive(EHttpdReqGetStateAsync, args, aStatus);
        }


    EXPORT_C TInt RHttpd::GetStateCancel() const
        {
        return SendReceive(EHttpdReqGetStateAsyncCancel, TIpcArgs());
        }


    EXPORT_C TInt RHttpd::SetSettings(const TSettings& aSettings) const
        {
        TPckgC<THttpdSettings>
            arg(aSettings);
        TIpcArgs
            args;
        
        args.Set(1, &arg);
        
        return SendReceive(EHttpdReqSetSettings, args);
        }


    EXPORT_C TInt RHttpd::GetSettings(TSettings& aSettings) const
        {
        TPckg<THttpdSettings>
            arg(aSettings);
        TIpcArgs
            args;
        
        args.Set(0, &arg);
        
        return SendReceive(EHttpdReqGetSettings, args);
        }


    EXPORT_C TInt RHttpd::GetCurrentSettings(TSettings& aSettings) const
        {
        TPckg<THttpdSettings>
            arg(aSettings);
        TIpcArgs
            args;
        
        args.Set(0, &arg);
        
        return SendReceive(EHttpdReqGetCurrentSettings, args);
        }
}
