#ifndef httpdui_cdocument_h
#define httpdui_cdocument_h
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

#include "httpdui/private.h"
#include <akndoc.h>
#include "mws/chttpd.h"
#include "mws/carguments.h"

class CWLAN;
   

namespace HttpdUi
{
    using Mws::CHttpd;


    class CDocument : public CAknDocument
        {
    public:
        static CDocument* NewL (CEikApplication& aApp);
        static CDocument* NewLC(CEikApplication& aApp);

        ~CDocument();

        Mws::CHttpd&               Httpd();
        const Mws::THttpdSettings& Settings() const;
        const Mws::CArguments&     Arguments() const;

        void SetSettingsL (const Mws::THttpdSettings& aSettings);
        void SetArgumentsL(const TDesC& aArguments);

        CWLAN& WLAN();

    private:
        CEikAppUi* CreateAppUiL();

    private:
        CDocument(CEikApplication& aApp);
        
        void ConstructL();
        
    private:
        Mws::CHttpd*        ipHttpd;
        Mws::THttpdSettings iSettings;
        Mws::CArguments*    ipArguments;
        CWLAN*              ipWLAN;
    };

} // namespace HttpdUi

#endif
