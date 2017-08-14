#ifndef pampui_cdocument_h
#define pampui_cdocument_h
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

#include "pampui/private.h"
#include <akndoc.h>
#include "mws/chttpd.h"
#include "mws/thttpdsettings.h"
#include "mws/carguments.h"
#include <mysql/carguments.h>
#include <mysql/cmysqld.h>

class CWLAN;
   

namespace PampUi
{
    
    class CMySQL;


    class CDocument : public CAknDocument
        {
    public:
        static CDocument* NewL (CEikApplication& aApp);
        static CDocument* NewLC(CEikApplication& aApp);

        ~CDocument();

        Mws::CHttpd&               Httpd();
        const Mws::THttpdSettings& HttpdSettings() const;
        const Mws::CArguments&     HttpdArguments() const;

        MySQL::CMysqld&          Mysqld();
        const MySQL::CArguments& MysqldArguments() const;
        
        CWLAN& WLAN();

        void SetHttpdSettingsL  (const Mws::THttpdSettings& aSettings);
        void SetHttpdArgumentsL (const TDesC&               aArguments);
        void SetMysqldArgumentsL(const TDesC&               aArguments);

    private:
        CEikAppUi* CreateAppUiL();

    private:
        CDocument(CEikApplication& aApp);
        
        void ConstructL();
        
    private:
        Mws::CHttpd*        ipHttpd;
        Mws::THttpdSettings iHttpdSettings;
        Mws::CArguments*    ipHttpdArguments;
        MySQL::CMysqld*     ipMysqld;
        MySQL::CArguments*  ipMysqldArguments;
        CWLAN*              ipWLAN;
    };

} // namespace PampUi

#endif
