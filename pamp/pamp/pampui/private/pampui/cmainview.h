#ifndef pampui_cmainview_h
#define pampui_cmainview_h
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
#include <aknview.h>


namespace PampUi
{
    
    class CMainContainer;
    class CDocument;
    
    
    class CMainView : public CAknView
        {
    public:
        enum
            {
            KId = EPampMainView
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
        CMainView(CDocument* apDocument);

        void ConstructL();

    private:
        void SetMenuL(TInt aResource);

        void ActivateWLANL();

        void StartHttpd(const TDesC& aArguments);
        void StartMysqld(const TDesC& aArguments);

    private:
        CDocument&      iDocument;
        CMainContainer* ipContainer;
        };
}
        
#endif
