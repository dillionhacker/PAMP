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

#include "httpds/chttpd.h"
#include "httpd/protocol.h"
#include "httpds/chttpdserver.h"

using namespace Mws;
using namespace Httpd;


namespace HttpdS
{
    CHttpd::CHttpd()
        {
        }


    CHttpd::~CHttpd()
        {
        Server()->Detach(this);
        }


    void CHttpd::ServiceL(const RMessage2& aMessage)
        {
        switch (aMessage.Function())
            {
            case EHttpdReqStart:
                StartServerL(aMessage);
                break;
                
            case EHttpdReqStop:
                StopServerL(aMessage);
                break;

            case EHttpdReqRestart:
                RestartServerL(aMessage);
                break;

            case EHttpdReqGetState:
                GetStateL(aMessage);
                break;
                
            case EHttpdReqGetStateAsync:            
                GetStateAsyncL(aMessage);
                break;
                
            case EHttpdReqGetStateAsyncCancel:
                GetStateAsyncCancelL(aMessage);
                break;
                
            case EHttpdReqStart2:
                StartServer2L(aMessage);
                break;
                
            case EHttpdReqSetSettings:
                SetSettingsL(aMessage);
                break;
                
            case EHttpdReqGetSettings:
                GetSettingsL(aMessage);
                break;

            case EHttpdReqGetCurrentSettings:
                GetCurrentSettingsL(aMessage);
                break;
        
            default:
                PanicClient(aMessage, EHttpdPanicBadRequest);
            }
        }
    


    void CHttpd::StartServerL(const RMessage2& aMessage)
        {
        THttpdSettings
            settings;
        TPckg<THttpdSettings>
            result(settings);

        aMessage.ReadL(1, result);

        aMessage.Complete(Server()->StartHttpdL(KNullDesC, settings));
        }

    
    void CHttpd::StopServerL(const RMessage2& aMessage)
        {
        TBool
            graceful = static_cast<TBool>(aMessage.Int1());

        aMessage.Complete(Server()->StopHttpd(graceful));
        }


    void CHttpd::RestartServerL(const RMessage2& aMessage)
        {
        TBool
            graceful = static_cast<TBool>(aMessage.Int1());

        aMessage.Complete(Server()->RestartHttpd(graceful));
        }


    void CHttpd::GetStateL(const RMessage2& aMessage)
        {
        StateRequest::ReturnL(aMessage, Server()->State());
        }
    

    void CHttpd::GetStateAsyncL(const RMessage2& aMessage)
        {
        if (iStateRequest.Pending())
            PanicClient(aMessage, EHttpdPanicRequestPending);
        else
            {
            TState
                state(Server()->State());
            
            if (state != iStateRequest.Data())
                {
                iStateRequest.Update(state);
                
                StateRequest::ReturnL(aMessage, state);
                }
            else
                iStateRequest.SetPending(aMessage);
            }
        }


    void CHttpd::GetStateAsyncCancelL(const RMessage2& aMessage)
        {
        if (iStateRequest.Pending())
            iStateRequest.Cancel(aMessage);
        else
            aMessage.Complete(KErrNone);
        }

    
    void CHttpd::StartServer2L(const RMessage2& aMessage)
        {
        THttpdSettings
            settings;
        TPckg<THttpdSettings>
            result(settings);

        aMessage.ReadL(1, result);
        
        TInt
            length = aMessage.GetDesLength(2);
        
        User::LeaveIfError(length);
        
        HBufC
            *pCommandLine = HBufC::NewLC(length);
        TPtr
            commandLine = pCommandLine->Des();
        
        aMessage.ReadL(2, commandLine);

        aMessage.Complete(Server()->StartHttpdL(commandLine, settings));
        
        CleanupStack::PopAndDestroy(pCommandLine);
        }


    void CHttpd::SetSettingsL(const RMessage2& aMessage)
        {
        THttpdSettings
            settings;
        TPckg<THttpdSettings>
            result(settings);

        aMessage.ReadL(1, result);

        aMessage.Complete(Server()->SetSettingsL(settings));
        }


    void CHttpd::GetSettingsL(const RMessage2& aMessage)
        {
        ReturnDataL(aMessage, Server()->Settings());
        }


    void CHttpd::GetCurrentSettingsL(const RMessage2& aMessage)
        {
        ReturnDataL(aMessage, Server()->CurrentSettings());
        }


    void CHttpd::UpdateStateL(TState aState)
        {
        if (iStateRequest.iPending)
            iStateRequest.ReturnL(aState);
        }


    void CHttpd::CreateL()
        {
        Server()->AttachL(this);
        }
    

    CHttpdServer* CHttpd::Server()
        {
        const CHttpd
            *pConstThis = this;
        const CHttpdServer
            *pConstServer = pConstThis->Server();

        return const_cast<CHttpdServer*>(pConstServer);
        }


    const CHttpdServer* CHttpd::Server() const
        {
        return static_cast<const CHttpdServer*>(CSession2::Server());
        }

}
