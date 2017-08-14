#ifndef httpdui_cmaincontainer_h
#define httpdui_cmaincontainer_h
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

#include "httpdui/private.h"
#include <coecntrl.h>
#include "mws/chttpd.h"
#include "cwlan.h"

class CEikLabel;


namespace HttpdUi
{

    class CMainContainer : public  CCoeControl, 
                           private MCoeControlObserver,
                           public  Mws::CHttpd::MObserver,
                           public  CWLAN::MObserver
        {
    public:
        typedef Mws::CHttpd::TState TState;

        static CMainContainer* NewL (const TRect& aRect);
        static CMainContainer* NewLC(const TRect& aRect);
        
        ~CMainContainer();

    private:
        // CoeControl
        void Draw(const TRect& aRect) const;

        TInt CountComponentControls() const;
        
        CCoeControl* ComponentControl(TInt aIndex) const;

    private:
        // MCoeControlObserver
        void HandleControlEventL(CCoeControl* aControl,
                                 TCoeEvent    aEventType);

        // CHttpd::MObserver
        void UpdateStateL(Mws::CHttpd* apHttpd, 
                          TState       aState);
        
        // CWLAN::MObserver
        void UpdateL(CWLAN*                        /*apWLAN*/,
                     const CWLAN::TConnectionStatus& aStatus);

    private:
        CMainContainer();
        
        void ConstructL(const TRect& aRect);

    private:
        CArrayPtr<CEikLabel>* ipLabels;
        CArrayPtr<CEikLabel>* ipTexts;
        CFbsBitmap*           ipBitmap;
    };

} // namespace HttpdUi

#endif
