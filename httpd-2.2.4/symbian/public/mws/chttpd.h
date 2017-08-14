#ifndef mws_chttpd_h
#define mws_chttpd_h
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

#include "mws/httpd.h"
#include <e32base.h>
#include "mws/mhttpdobserver.h"
#include "mws/rhttpd.h"


namespace Mws
{
    class CHttpd : public ::CBase
        {
    public:
        typedef MHttpdObserver MObserver;
        typedef THttpdSettings TSettings;
        typedef THttpdState    TState;

        IMPORT_C static CHttpd* NewL(TBool aAutoConnect = true);
        IMPORT_C static CHttpd* NewLC(TBool aAutoConnect = true);
        
        IMPORT_C ~CHttpd();

        IMPORT_C TVersion Version() const;

        IMPORT_C void ConnectL();
        IMPORT_C void Close();
        IMPORT_C void ReConnectL();
        
        IMPORT_C TBool Connected() const;
        
        IMPORT_C void StartL(const TSettings& aSettings = 
                             TSettings()) const;
        IMPORT_C void StartL(const TDesC&     aCommandLine,
                             const TSettings& aSettings = 
                             TSettings()) const;
        IMPORT_C void StopL(TBool aGraceful = true) const;
        IMPORT_C void RestartL(TBool aGraceful = true) const;
        
        IMPORT_C TState StateL() const;

        IMPORT_C void AttachL(MObserver* apObserver);
        IMPORT_C void Detach(MObserver* apObserver);

        IMPORT_C void      SetSettingsL(const TSettings& aSettings) const;
        IMPORT_C TSettings SettingsL() const;
        IMPORT_C TSettings CurrentSettingsL() const;

    private:
        CHttpd();

        void ConstructL(TBool aAutoConnect);

        CHttpd(const CHttpd&);
        CHttpd& operator = (const CHttpd&);

    private:
        class CImp;
        
        CImp* ipImp;
    };
}

#endif
