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

#include "mws/chttpdsettingsview.h"
#include "httpdutil/private.h"
#include <aknviewappui.h>
#include <bautils.h> 
#include <eikmenup.h>
#include "httpdutil/clistsettings.h"
#include "httpdutil/csettingslist.h"


namespace
{
    _LIT(KResourcePath, "\\resource\\apps\\httpdutil.rsc");
}


namespace Mws
{

    using namespace HttpdUtil;


    CHttpdSettingsView::CHttpdSettingsView(
        TInt                  aId,
        MHandler*             apHandler,
        const THttpdSettings* apDefaultSettings) : 
        iId(aId),
        iHandler(*apHandler),
        iDefaultSettings(*apDefaultSettings),
        iResource(-1)
        {
        }


    EXPORT_C
    CHttpdSettingsView::~CHttpdSettingsView()
        {
        DoDeactivate();
        
        delete ipSettings;

        if (iResource != -1)
            CEikonEnv::Static()->DeleteResourceFile(iResource);            
        }
    
    
    EXPORT_C
    CHttpdSettingsView* CHttpdSettingsView::NewL(
        TInt                  aId,
        MHandler*             apHandler,
        const THttpdSettings* apDefaultSettings)
        {
        CHttpdSettingsView
            *pThis = NewLC(aId, apHandler, apDefaultSettings);

        CleanupStack::Pop(pThis);

        return pThis;
        }


    EXPORT_C
    CHttpdSettingsView* CHttpdSettingsView::NewLC(
        TInt                  aId,
        MHandler*             apHandler,
        const THttpdSettings* apDefaultSettings)
        {
        CHttpdSettingsView
            *pThis = new (ELeave) CHttpdSettingsView(aId,
                                                     apHandler, 
                                                     apDefaultSettings);
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL();
        
        return pThis;
        }
        

    void CHttpdSettingsView::ConstructL()
        {
        CEikonEnv
            *pEnv = CEikonEnv::Static();
        
        TFileName
            path(KResourcePath);

        CompleteWithAppPath(path);

        BaflUtils::NearestLanguageFile(pEnv->FsSession(), path);
        
        iResource = pEnv->AddResourceFileL(path);
        
        BaseConstructL(R_HTTPDUTIL_SETTINGS_VIEW);

        ipSettings = CListSettings::NewL();
        }


    
    TUid CHttpdSettingsView::Id() const
        {
        return TUid::Uid(iId);
        }


    void CHttpdSettingsView::HandleCommandL(TInt aCommand)
        {
        switch (aCommand)
            {
            case EAknSoftkeyOk:
                {
                THttpdSettings
                    settings;

                ipContainer->StoreSettingsL();
                ipSettings->CopyTo(settings);

                iHandler.SaveL(this, settings);
                }
                break;
                
            case EAknSoftkeyCancel:
                iHandler.CancelL(this);
                break;
                
            default:
                AppUi()->HandleCommandL(aCommand);
            }
        }


    void CHttpdSettingsView::HandleClientRectChange()
        {
        if (ipContainer)
            ipContainer->SetRect(ClientRect());
        }

    
    void CHttpdSettingsView::DoActivateL(const TVwsViewId& /* aPrevViewId */,
                                         TUid              /* aCustomMsgId*/,
                                         const TDesC8&     /* aCustomMsg*/)
        {
        if (!ipContainer)
            {
            ipSettings->CopyFromL(iDefaultSettings);
            
            ipContainer = CSettingsList::NewL(ipSettings);
            ipContainer->SetMopParent(this);
            
            AppUi()->AddToStackL(*this, ipContainer);
            }
        }
    
    
    void CHttpdSettingsView::DoDeactivate()
        {
        if (ipContainer)
            {
            AppUi()->RemoveFromViewStack(*this, ipContainer);
            
            delete ipContainer;
            ipContainer = 0;
            }
        }

} // namespace Mws
