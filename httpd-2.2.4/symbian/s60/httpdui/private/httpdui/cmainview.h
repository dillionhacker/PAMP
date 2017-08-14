#ifndef httpdui_cmainview_h
#define httpdui_cmainview_h
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
#include <aknview.h>
#include "mws/chttpd.h"


namespace HttpdUi
{
    class CMainContainer;
    class CEngine;
    class CDocument;
    
    
    class CMainView : public  CAknView,
                      private Mws::CHttpd::MObserver
        {
    public:
        enum
            {
            KId = KIdMainView
            };
        
        static CMainView* NewL (CDocument* apDocument);
        static CMainView* NewLC(CDocument* apDocument);
        
        ~CMainView();

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
        // MEikMenuObserver
        void DynInitMenuPaneL(TInt aMenuID, CEikMenuPane* apMenuPane);

    private:
        // Mws::CHttpd::MObserver
        void UpdateStateL(Mws::CHttpd*        apHttpd,
                          Mws::CHttpd::TState aState);

    private:
        CMainView(CDocument* apDocument);

        void ConstructL();

    private:
        void SetMenuL(TInt aResource);
        
    private:
        CDocument&      iDocument;
        CMainContainer* ipContainer;
        };

}

#endif
