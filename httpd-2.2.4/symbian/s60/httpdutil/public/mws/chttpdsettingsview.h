#ifndef mws_chttpdsettingsview_h
#define mws_chttpdsettingsview_h
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

#include "mws/httpdutil.h"
#include <aknview.h>
#include "mws/thttpdsettings.h"
#include "mws/mhttpdsettingsviewhandler.h"


namespace HttpdUtil
{
    class CListSettings;
    class CSettingsList;
}


namespace Mws
{

    class CHttpdSettingsView : public CAknView
        {
    public:
        typedef MHttpdSettingsViewHandler MHandler;
        
        IMPORT_C static 
        CHttpdSettingsView* NewL (TInt                  aId,
                                  MHandler*             apHandler, 
                                  const THttpdSettings* apDefaultSettings);
        IMPORT_C static 
        CHttpdSettingsView* NewLC(TInt                  aId,
                                  MHandler*             apHandler, 
                                  const THttpdSettings* apDefaultSettings);
        
        IMPORT_C ~CHttpdSettingsView();

    public:
        // CAknView
        TUid Id() const;

    private:
        // CAknView
        void HandleCommandL(TInt aCommand);

        void HandleClientRectChange();

        void DoActivateL(const TVwsViewId& aPrevViewId,
                         TUid              aCustomMessageId,
                         const TDesC8&     aCustomMessage);

        void DoDeactivate();

    private:
        CHttpdSettingsView(TInt                  aId,
                           MHandler*             apHandler,
                           const THttpdSettings* apDefaultSettings);

        void ConstructL();

    private:
        TInt                      iId;
        MHandler&                 iHandler;
        const THttpdSettings&     iDefaultSettings;
        HttpdUtil::CListSettings* ipSettings;
        HttpdUtil::CSettingsList* ipContainer;
        TInt                      iResource;
        };

} // namespace Mws

#endif
