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

#include "httpdutil/clistsettings.h"


namespace HttpdUtil
{
    CListSettings::~CListSettings()
        {
        }

    
    CListSettings* CListSettings::NewL()
        {
        CListSettings
            *pThis = NewLC();
        
        CleanupStack::Pop();

        return pThis;
        }
    
    
    CListSettings* CListSettings::NewLC()
        {
        CListSettings
            *pThis = new (ELeave) CListSettings;

        pThis->ConstructL();
        
        CleanupStack::PushL(pThis);
        
        return pThis;
        }


    
    void CListSettings::CopyFromL(const Mws::THttpdSettings& aSettings)
        {
        iStackSize   = aSettings.StackSize();
        iMinHeapSize = aSettings.MinHeapSize();
        iMaxHeapSize = aSettings.MaxHeapSize();
        }


    void CListSettings::CopyTo(Mws::THttpdSettings& aSettings) const
        {
        aSettings.SetStackSize(iStackSize);
        aSettings.SetMinHeapSize(iMinHeapSize);
        aSettings.SetMaxHeapSize(iMaxHeapSize);
        }


    void CListSettings::ConstructL()
        {
        }

} // namespace HttpdUtil
