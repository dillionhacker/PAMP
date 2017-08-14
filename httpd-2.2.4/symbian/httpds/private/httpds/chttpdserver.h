#ifndef httpds_cserver_h
#define httpds_cserver_h
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

#include "httpds/private.h"
#include <e32base.h>
#include "mws/thttpdsettings.h"
#include "mws/thttpdstate.h"


namespace HttpdS
{
    class CHttpd;
    
    
    class CHttpdServer : public ::CServer2
        {
    public:
        typedef Mws::THttpdSettings TSettings;
        typedef Mws::THttpdState    TState;
        
        ~CHttpdServer();
    
        static CHttpdServer* NewL (TInt aPriority);
        static CHttpdServer* NewLC(TInt aPriority);
        
        static void PanicServer(THttpdPanic aReason);

        void AttachL(CHttpd* apSession);
        void Detach (CHttpd* apSession);

    public:
        TInt StartHttpdL (const TDesC&     aCommandLine,
                          const TSettings& aSettings);
        TInt StopHttpd   (TBool aGraceful);
        TInt RestartHttpd(TBool aGraceful);

        TState State() const;

        TInt      SetSettingsL(const TSettings& aSettings);
        TSettings Settings() const;
        TSettings CurrentSettings() const;

    protected:
        TInt RunError(TInt aError);
        
        ::CSession2* NewSessionL(const TVersion&  aVersion,
                                 const RMessage2& aMessage) const;
        
    private:
        CHttpdServer(TInt aPriority);
        
        void ConstructL();

        void ReadSettingsL();
        
    public:
        class CPoller;
        class CThreadWaiter;
        class CWorkerOverseer;
        
        friend class CWaiter;
        
        void UpdateHttpdState(TState aState);

        void WorkerCreated(TThreadId aThreadId);

        void ThreadDeath(TThreadId aThreadId,
                         TExitType aExitType,
                         TInt      aExitReason);
            
    private:
        CArrayPtr<CHttpd>*            ipSessions;
        TState                        iState;
        RThread                       iThread;
        CPoller*                      ipPoller;
        CThreadWaiter*                ipThreadWaiter;
        TBool                         iShouldExit;
        TFileName                     iPrivatePath;
        TSettings                     iSettings;
        TSettings                     iCurrentSettings;
        CWorkerOverseer*              ipWorkerOverseer;
        RSemaphore                    iSemaphore;
        CArrayPtrFlat<CThreadWaiter>* ipWaiters;
        };
}

#endif
