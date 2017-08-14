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

#include "httpdutil/csettingslist.h"
#include <barsread.h>
#include "httpdutil/clistsettings.h"


namespace HttpdUtil
{

    CSettingsList::CSettingsList(CListSettings* pData)
        : iData(*pData)
        {
        }


    CSettingsList::~CSettingsList()
        {
        }


        
    CSettingsList* CSettingsList::NewL(CListSettings* pData)
        {
        CSettingsList
            *pThis = NewLC(pData);

        CleanupStack::Pop();

        return pThis;
        }


    CSettingsList* CSettingsList::NewLC(CListSettings* pData)
        {
        CSettingsList
            *pThis = new (ELeave) CSettingsList(pData);

        CleanupStack::PushL(pThis);
        
        pThis->ConstructL();
        
        return pThis;
        }
        

    void CSettingsList::ConstructL()
        {
        ConstructFromResourceL(R_HTTPDUTIL_SETTINGS);
        ActivateL();
        }


    void CSettingsList::SizeChange()
        {
        CEikFormattedCellListBox
            *pListBox = ListBox();
        
        if (pListBox)
            pListBox->SetRect(Rect());
        }
        
    
    void CSettingsList::EditCurrentItemL()
        {
        EditItemL(ListBox()->CurrentItemIndex(), ETrue);
        }

    
    CAknSettingItem* CSettingsList::CreateSettingItemL(TInt aId)
        {
        CAknSettingItem
            *pItem = 0;
        
        switch (aId)
            {
            case ESettingStackSize:
                {
                TInt
                    &data = iData.StackSize();
                
                pItem = new (ELeave) CAknIntegerEdwinSettingItem(aId, data);
                }
                break;

            case ESettingMinHeapSize:
                {
                TInt
                    &data = iData.MinHeapSize();
                
                pItem = new (ELeave) CAknIntegerEdwinSettingItem(aId, data);
                }
                break;

            case ESettingMaxHeapSize:
                {
                TInt
                    &data = iData.MaxHeapSize();
                
                pItem = new (ELeave) CAknIntegerEdwinSettingItem(aId, data);
                }
                break;
            }

        return pItem;
        }

} // namespace HttpdUtil
