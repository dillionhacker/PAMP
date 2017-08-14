#ifndef mws_mhttpdsettingsviewhandler_h
#define mws_mhttpdsettingsviewhandler_h
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
#include "mws/thttpdsettings.h"


namespace Mws
{

    class CHttpdSettingsView;
    
    
    class MHttpdSettingsViewHandler
        {
    public:
        IMPORT_C virtual void SaveL  (CHttpdSettingsView*   apView, 
                                      const THttpdSettings& aSettings) = 0;
        
        IMPORT_C virtual void CancelL(CHttpdSettingsView* apView) = 0;
        };

} // namespace Mws

#endif
