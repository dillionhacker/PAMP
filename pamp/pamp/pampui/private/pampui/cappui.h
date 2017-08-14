#ifndef pampui_cappui_h
#define pampui_cappui_h
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
#include <aknviewappui.h>
#include "mws/chttpdsettingsview.h"


namespace PampUi
{

    class CDocument;
    
    
    class CAppUi : public  CAknViewAppUi,
                   private Mws::CHttpdSettingsView::MHandler
        {
    public:
        CAppUi(CDocument* apDocument);
        ~CAppUi();
        
        void ConstructL();
    
    private:
        // CEikAppUi
        void HandleCommandL(TInt aCommand);

    private:
        // Mws::CHttpdSettingsView::MHandler
        void SaveL  (Mws::CHttpdSettingsView*   apView, 
                     const Mws::THttpdSettings& aSettings);
        void CancelL(Mws::CHttpdSettingsView*   apView);

    private:
        CDocument& iDocument;
        };

} // namespace PampUi

#endif
