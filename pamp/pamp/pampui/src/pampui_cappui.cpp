/* Copyright 2007 Nokia Corporation
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

#include "pampui/cappui.h"
#include <avkon.hrh>
#include <aknutils.h>
#include <bautils.h>
#include <eikmenup.h>
#include "pampui/cdocument.h"
#include "pampui/cmainview.h"


namespace PampUi
{
    using namespace Mws;
    using namespace MySQL;


    CAppUi::CAppUi(CDocument* apDocument) :
        iDocument(*apDocument)
        {
        }


    CAppUi::~CAppUi()
        {
       }
    

    void CAppUi::ConstructL()
        {
        BaseConstructL(EAknEnableSkin);
        
        CMainView
            *pView = CMainView::NewL(&iDocument);
        
        AddViewL(pView); // Ownership moved.
        
        SetDefaultViewL(*pView);
        }


    void CAppUi::HandleCommandL(TInt aCommand)
        {
        switch (aCommand)
            {
            case EAknSoftkeyBack:
            case EAknSoftkeyExit:
            case EEikCmdExit:
                Exit();
                break;

            case ECmdSettingsHttpd:
                {
                CAknView
                    *pView = View(TUid::Uid(EHttpdSettingsView));
                
                if (!pView)
                    {
                    pView = CHttpdSettingsView::NewL(
                        EHttpdSettingsView,
                        this, 
                        &iDocument.HttpdSettings());
                    
                    AddViewL(pView); // Ownership moved.
                    }

                ActivateLocalViewL(TUid::Uid(EHttpdSettingsView));
                }
                break;

            default:
                break;      
            }
        }


    void CAppUi::SaveL(CHttpdSettingsView*, 
                       const THttpdSettings& aSettings)
        {
        iDocument.SetHttpdSettingsL(aSettings);
        
        ActivateLocalViewL(TUid::Uid(CMainView::KId));
        }


    void CAppUi::CancelL(CHttpdSettingsView*)
        {
        ActivateLocalViewL(TUid::Uid(CMainView::KId));
        }

} // namespace PampUi
