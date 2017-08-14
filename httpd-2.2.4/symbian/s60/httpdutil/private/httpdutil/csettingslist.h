#ifndef httpdutil_csettingslist_h
#define httpdutil_csettingslist_h
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

#include "httpdutil/private.h"
#include <aknsettingitemlist.h>


namespace HttpdUtil
{

    class CListSettings;
    
    
    NONSHARABLE_CLASS(CSettingsList) : public CAknSettingItemList
        {
    public:
        ~CSettingsList();
        
        static CSettingsList* NewL (CListSettings* pData);
        static CSettingsList* NewLC(CListSettings* pData);
        
        void SizeChange();
        
        void EditCurrentItemL();

    protected:
        CAknSettingItem* CreateSettingItemL(TInt aSettingId);
        
    private:
        CSettingsList(CListSettings* pData);
        
        void ConstructL();
        
    private:
        CListSettings& iData;
        };

} // namespace HttpdUtil

#endif
