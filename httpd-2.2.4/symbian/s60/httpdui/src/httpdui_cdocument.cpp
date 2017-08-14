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

#include "httpdui/cdocument.h"
#include <aknglobalnote.h> 
#include "httpdui/cappui.h"
#include "cwlan.h"


namespace
{
    _LIT(KNoHttpd, "C:\\Data\\Apache does not exist.");
}



namespace HttpdUi
{

    CDocument::CDocument(CEikApplication& aApp) :
        CAknDocument(aApp)    
        {
        }
    

    CDocument::~CDocument()
        {
        delete ipWLAN;
        delete ipArguments;
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


    void CDocument::ConstructL()
        {
        ipHttpd     = Mws::CHttpd::NewL();
        ipArguments = Mws::CArguments::NewL();
        ipWLAN      = CWLAN::NewL();

        iSettings = ipHttpd->SettingsL();

        TRAPD(rc, 
              ipArguments->InternalizeL(Mws::KDefaultHttpdArgumentsFileName));
        
        switch (rc)
            {
            case KErrNone:
            case KErrNotFound:
                break;
                
            case KErrPathNotFound:
                {
                CAknGlobalNote
                    *pNote = CAknGlobalNote::NewLC();
                
                pNote->ShowNoteL(EAknGlobalErrorNote, KNoHttpd);
                
                CleanupStack::PopAndDestroy(pNote);
                }
                break;

            default:
                User::Leave(rc);
            }
        }


    Mws::CHttpd& CDocument::Httpd()
        {
        return *ipHttpd;
        }

    
    const Mws::THttpdSettings& CDocument::Settings() const
        {
        return iSettings;
        }


    const Mws::CArguments& CDocument::Arguments() const
        {
        return *ipArguments;
        }
    

    void CDocument::SetSettingsL(const Mws::THttpdSettings& aSettings)
        {
        ipHttpd->SetSettingsL(aSettings);

        iSettings = aSettings;
        }
    

    void CDocument::SetArgumentsL(const TDesC& aString)
        {
        ipArguments->SetL(aString);

        ipArguments->ExternalizeL(Mws::KDefaultHttpdArgumentsFileName);
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
