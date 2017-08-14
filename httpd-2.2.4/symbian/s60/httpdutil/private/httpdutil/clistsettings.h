#ifndef httpdutil_clistsettings_h
#define httpdutil_clistsettings_h
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
#include "mws/thttpdsettings.h"


namespace HttpdUtil
{

    NONSHARABLE_CLASS(CListSettings) : public CBase
        {
    public:
        ~CListSettings();
        
        static CListSettings* NewL();
        static CListSettings* NewLC();
        
        TInt& StackSize()
            {
            return iStackSize;
            }

        TInt StackSize() const
            {
            return iStackSize;
            }

        TInt& MinHeapSize()
            {
            return iMinHeapSize;
            }

        TInt MinHeapSize() const
            {
            return iMinHeapSize;
            }

        TInt& MaxHeapSize()
            {
            return iMaxHeapSize;
            }

        TInt MaxHeapSize() const
            {
            return iMaxHeapSize;
            }

        void CopyFromL(const Mws::THttpdSettings& aSettings);
        void CopyTo   (Mws::THttpdSettings& aSettings) const;

    private:
        void ConstructL();

    private:
        TInt iStackSize;
        TInt iMinHeapSize;
        TInt iMaxHeapSize;
        };

} // namespace HttpdUtil

#endif
