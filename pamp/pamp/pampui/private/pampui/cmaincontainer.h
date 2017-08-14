#ifndef pampui_cmaincontainer_h
#define pampui_cmaincontainer_h
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

#include "pampui/private.h"
#include <coecntrl.h>
#include <mws/chttpd.h>
#include <mysql/cmysqld.h>
#include "cwlan.h"

class CCommsDatabase;
class CApUtils;
class CEikLabel;


namespace PampUi
{
    
    class CMainContainer : public  CCoeControl,
                           private MCoeControlObserver,
                           public  Mws::CHttpd::MObserver,
                           public  MySQL::CMysqld::MObserver,
                           public  CWLAN::MObserver
        {
    public:
        static CMainContainer* NewL (const TRect& aRect);
        static CMainContainer* NewLC(const TRect& aRect);

        ~CMainContainer();
        
    public:
        // CCoeControl
        void Draw(const TRect& aRect) const;
        
        TInt CountComponentControls() const;
        
        CCoeControl* ComponentControl(TInt aIndex) const;

    private:
        // MCoeControlObserver
        void HandleControlEventL(CCoeControl* aControl,
                                 TCoeEvent    aEventType);
        
    private:
        // CHttpd::MObserver
        void UpdateStateL(Mws::CHttpd*        apHttpd, 
                          Mws::CHttpd::TState aState);

        // MySQL::CMysqld::MObserver
        void UpdateStateL(MySQL::CMysqld*        apMySQL, 
                          MySQL::CMysqld::TState aState);

        // CWLAN::MObserver
        void UpdateL(CWLAN*                          apWLAN,
                     const CWLAN::TConnectionStatus& aStatus);

    private:
        CMainContainer();
        
        void ConstructL(const TRect& aRect);

    private:
        CArrayPtr<CEikLabel>* ipLabels;
        CArrayPtr<CEikLabel>* ipTexts;
        CFbsBitmap*           ipApache;
        CFbsBitmap*           ipMySQL;
        CFbsBitmap*           ipPHP;
        };


} // namespace PampUi

#endif
