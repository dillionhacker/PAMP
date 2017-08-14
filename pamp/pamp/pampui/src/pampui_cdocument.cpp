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

#include "pampui/cdocument.h"
#include <aknglobalnote.h>
#include <mysql/mysqldutil.h>
#include "mws/httpdutil.h" 
#include "pampui/cappui.h"
#include "cwlan.h"


namespace
{
    _LIT(KNoHttpd, "C:\\Data\\Apache does not exist.");
    _LIT(KNoMySQL, "C:\\Data\\MySQL does not exist.");
}


namespace PampUi
{
    using namespace Mws;
    using namespace MySQL;


    CDocument::CDocument(CEikApplication& aApp) :
        CAknDocument(aApp)    
        {
        }
    

    CDocument::~CDocument()
        {
        delete ipWLAN;
        delete ipMysqldArguments;
        delete ipMysqld;
        delete ipHttpdArguments;
        delete ipHttpd;
        }


    CDocument* CDocument::NewL(CEikApplication& aApp)
        {
        CDocument 
            *pThis = NewLC(aApp);
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }


    CDocument* CDocument::NewLC(CEikApplication& aApp)
        {
        CDocument 
            *pThis = new (ELeave) CDocument(aApp);
        
        CleanupStack::PushL(pThis);

        pThis->ConstructL();
        
        return pThis;
        }


    namespace
        {
            void CheckInternalizationL(TInt rc, const TDesC& aMessage)
                {
                switch (rc)
                    {
                    case KErrNone:
                    case KErrNotFound:
                        break;
                        
                    case KErrPathNotFound:
                        {
                        CAknGlobalNote
                            *pNote = CAknGlobalNote::NewLC();
                        
                        pNote->ShowNoteL(EAknGlobalErrorNote, aMessage);
                        
                        CleanupStack::PopAndDestroy(pNote);
                        }
                        break;
                        
                    default:
                        User::Leave(rc);
                    }
                }
        }


    void CDocument::ConstructL()
        {
        ipHttpd           = CHttpd::NewL();
        ipHttpdArguments  = Mws::CArguments::NewL();
        ipMysqld          = CMysqld::NewL();
        ipMysqldArguments = MySQL::CArguments::NewL();
        ipWLAN            = CWLAN::NewL();

        iHttpdSettings = ipHttpd->SettingsL();

        TInt
            rc;
        
        TRAP(rc, ipHttpdArguments->InternalizeL(Mws::KDefaultHttpdArgumentsFileName));
        CheckInternalizationL(rc, KNoHttpd);
        
        TRAP(rc, ipMysqldArguments->InternalizeL(MySQL::KDefaultMysqldArgumentsFileName));
        CheckInternalizationL(rc, KNoMySQL);
        }


    CHttpd& CDocument::Httpd()
        {
        return *ipHttpd;
        }

    
    const Mws::THttpdSettings& CDocument::HttpdSettings() const
        {
        return iHttpdSettings;
        }

    
    const Mws::CArguments& CDocument::HttpdArguments() const
        {
        return *ipHttpdArguments;
        }
    

    CMysqld& CDocument::Mysqld()
        {
        return *ipMysqld;
        }


    const MySQL::CArguments& CDocument::MysqldArguments() const
        {
        return *ipMysqldArguments;
        }


    void CDocument::SetHttpdSettingsL(const THttpdSettings& aSettings)
        {
        ipHttpd->SetSettingsL(aSettings);

        iHttpdSettings = aSettings;
        }
    

    void CDocument::SetHttpdArgumentsL(const TDesC& aArguments)
        {
        ipHttpdArguments->SetL(aArguments);
        
        ipHttpdArguments->ExternalizeL(Mws::KDefaultHttpdArgumentsFileName);
        }
    

    void CDocument::SetMysqldArgumentsL(const TDesC& aArguments)
        {
        ipMysqldArguments->SetL(aArguments);
        
        ipMysqldArguments->ExternalizeL(MySQL::KDefaultMysqldArgumentsFileName);
        }
    

    CWLAN& CDocument::WLAN()
        {
        return *ipWLAN;
        }

    
    CEikAppUi* CDocument::CreateAppUiL()
        {
        return new (ELeave) CAppUi(this);
        }

} // namespace HttpdUi
