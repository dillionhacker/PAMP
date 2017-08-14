#ifndef httpds_chttpd_h
#define httpds_chttpd_h
/* Copyright 2008 Nokia Corporation
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

#include "httpds/private.h"
#include "mws/thttpdstate.h"
#include "httpds/request.h"


namespace HttpdS
{
    class CHttpdServer;

    
    class CHttpd : public ::CSession2
        {
    public:
        typedef Mws::THttpdState TState;
        
        CHttpd();

        ~CHttpd();
        
    private:
        void ServiceL(const RMessage2& aMessage);

    private:
        // REQUESTS
        void StartServerL(const RMessage2& aMessage);
        void StopServerL(const RMessage2& aMessage);
        void RestartServerL(const RMessage2& aMessage);

        void GetStateL(const RMessage2& aMessage);
        void GetStateAsyncL(const RMessage2& aMessage);
        void GetStateAsyncCancelL(const RMessage2& aMessage);

        void StartServer2L(const RMessage2& aMessage);
        
        void SetSettingsL(const RMessage2& aMessage);
        void GetSettingsL(const RMessage2& aMessage);
        void GetCurrentSettingsL(const RMessage2& aMessage);

    public:
        void UpdateStateL(TState aState);
        
    private:
        // Called by C/S framework.
        void CreateL();

    private:
        CHttpdServer*       Server();
        const CHttpdServer* Server() const;
        
    public:
        typedef AsyncRequest<TState> StateRequest;
        
    private:
        StateRequest iStateRequest;
        };
}

#endif
