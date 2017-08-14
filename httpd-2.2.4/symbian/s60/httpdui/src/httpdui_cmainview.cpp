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

#include "httpdui/cmainview.h"
#include <aknnotedialog.h>
#include <aknquerydialog.h> 
#include <aknviewappui.h>
#include <aknutils.h>
#include <avkon.rsg>
#include <bautils.h>
#include <eikbtgpc.h>
#include <eikmenup.h>
#include "mws/cargumentsquerydialog.h"
#include "httpdui/cmaincontainer.h"
#include "httpdui/cdocument.h"
#include "cwlan.h"

using namespace Mws;


namespace HttpdUi
{

    CMainView::CMainView(CDocument* apDocument) :
        iDocument(*apDocument)
        {
        }
    
    
    CMainView::~CMainView()
        {
        iDocument.Httpd().Detach(this);
        
        DoDeactivate();
        }
    
    
    CMainView* CMainView::NewL(CDocument* apDocument)
        {
        CMainView
            *pThis = NewLC(apDocument);

        CleanupStack::Pop(pThis);

        return pThis;
        }


    CMainView* CMainView::NewLC(CDocument* apDocument)
        {
        CMainView
            *pThis = new (ELeave) CMainView(apDocument);

        CleanupStack::PushL(pThis);
        pThis->ConstructL();

        return pThis;
        }
        

    void CMainView::ConstructL()
        {
        BaseConstructL(R_HTTPDUI_MAIN_VIEW);

        iDocument.Httpd().AttachL(this);
        }


    
    TUid CMainView::Id() const
        {
        return TUid::Uid(KId);
        }
    

    void CMainView::HandleCommandL(TInt aCommand)
        {
        switch (aCommand)
            {
            case EAknSoftkeyBack:
            case EAknSoftkeyExit:
                {
                // Has to be caught, otherwise there would be
                // no exit on error.
                Mws::CHttpd
                    &httpd = iDocument.Httpd();

                if (httpd.Connected())
                    {
                    TRAP_IGNORE(httpd.StopL());
                    }
                
                AppUi()->HandleCommandL(aCommand);
                }
                break;

            case ECmdStart:
                {
                const TDesC
                    &arguments = iDocument.Arguments().Get();
                TInt
                    length = Max(arguments.Length(), KHttpdMaxArgumentsLength);
                HBufC
                    *pArguments = HBufC::NewLC(length);

                *pArguments = arguments;

                TPtr
                    buffer = pArguments->Des();

                CArgumentsQueryDialog::TButtons
                    buttons = CArgumentsQueryDialog::EOkCancel;
                CArgumentsQueryDialog
                    *pDialog = CArgumentsQueryDialog::NewL(buffer, buttons);
                
                if (pDialog->RunLD() == EAknSoftkeyOk)
                    {
                    iDocument.Httpd().StartL(*pArguments);

                    CWLAN
                        &wlan = iDocument.WLAN();
                    
                    if (!wlan.ActivatedL())
                        {
                        TInt
                            id;
                        
                        if (wlan.ActiveL())
                            id = R_HTTPDUI_ATTACH_WLAN;
                        else
                            id = R_HTTPDUI_START_WLAN;
                        
                        CAknNoteDialog
                            *pDialog = new (ELeave) CAknNoteDialog;
                        
                        if (pDialog->ExecuteLD(id))
                            wlan.ActivateL();
                        }
                    }
                }
                break;


            case ECmdArguments:
                {
                const TDesC
                    &arguments = iDocument.Arguments().Get();
                TInt
                    length = Max(arguments.Length(), KHttpdMaxArgumentsLength);
                HBufC
                    *pArguments = HBufC::NewLC(length);
                
                *pArguments = arguments;
                
                TPtr
                    buffer = pArguments->Des();

                CArgumentsQueryDialog::TButtons
                    buttons = CArgumentsQueryDialog::ESaveCancel;
                CArgumentsQueryDialog
                    *pDialog = CArgumentsQueryDialog::NewL(buffer, buttons);

                if (pDialog->RunLD() == EAknSoftkeySave)
                    iDocument.SetArgumentsL(*pArguments);

                CleanupStack::PopAndDestroy(pArguments);
                }
                break;
                

            case ECmdRestart:
                iDocument.Httpd().RestartL();
                break;
                
            case ECmdStop:
                iDocument.Httpd().StopL();
                break;

            case ECmdReset:
                iDocument.Httpd().ReConnectL();
                break;

            case ECmdStartWLAN:
            case ECmdAttachWLAN:
                iDocument.WLAN().ActivateL();
                break;

            case ECmdCloseWLAN:
                iDocument.WLAN().Close();
                break;

            default:
                AppUi()->HandleCommandL(aCommand);
            }
        }

    
    void CMainView::HandleClientRectChange()
        {
        if (ipContainer)
            ipContainer->SetRect(ClientRect());
        }

    
    void CMainView::DoActivateL(const TVwsViewId& /* aPrevViewId */,
                                TUid              /* aCustomMessageId */,
                                const TDesC8&     /* aCustomMessage*/)
        {
        if (!ipContainer)
            {
            ipContainer = CMainContainer::NewL(ClientRect());
            ipContainer->SetMopParent(this);

            iDocument.Httpd().AttachL(ipContainer);
            iDocument.WLAN().AttachL(ipContainer);
            
            AppUi()->AddToStackL(*this, ipContainer);
            }
        }

    
    void CMainView::DoDeactivate()
        {
        if (ipContainer)
            {
            AppUi()->RemoveFromViewStack(*this, ipContainer);

            iDocument.Httpd().Detach(ipContainer);
            iDocument.WLAN().Detach(ipContainer);
            
            delete ipContainer;
            ipContainer = 0;
            }
        }


    void CMainView::UpdateStateL(CHttpd*, CHttpd::TState aState)
        {
        switch (aState)
            {
            case Mws::EHttpdNotStarted:
            case Mws::EHttpdStopped:
            case Mws::EHttpdTerminated:
                SetMenuL(R_AVKON_SOFTKEYS_OPTIONS_EXIT);
                break;
                
            default:
                SetMenuL(R_HTTPDUI_SOFTKEYS_OPTIONS);
                break;
            }
        }


    void CMainView::DynInitMenuPaneL(TInt          aMenuId, 
                                           CEikMenuPane* apMenuPane)
        {
        if (aMenuId == R_HTTPDUI_MAIN_MENU)
            {
            Mws::THttpdState
                state = Mws::EHttpdTerminated;
            
            TRAP_IGNORE(state = iDocument.Httpd().StateL());
            
            switch (state)
                {
                case Mws::EHttpdNotStarted:
                    apMenuPane->SetItemDimmed(ECmdStart,   EFalse);
                    apMenuPane->SetItemDimmed(ECmdRestart, ETrue);
                    apMenuPane->SetItemDimmed(ECmdStop,    ETrue);
                    apMenuPane->SetItemDimmed(ECmdReset,   ETrue);
                    break;

                case Mws::EHttpdStarting:
                    apMenuPane->SetItemDimmed(ECmdStart,   ETrue);
                    apMenuPane->SetItemDimmed(ECmdRestart, ETrue);
                    apMenuPane->SetItemDimmed(ECmdStop,    EFalse);
                    apMenuPane->SetItemDimmed(ECmdReset,   ETrue);
                    break;

                case Mws::EHttpdRunning:
                    apMenuPane->SetItemDimmed(ECmdStart,   ETrue);
                    apMenuPane->SetItemDimmed(ECmdRestart, EFalse);
                    apMenuPane->SetItemDimmed(ECmdStop,    EFalse);
                    apMenuPane->SetItemDimmed(ECmdReset,   ETrue);
                    break;

                case Mws::EHttpdRestarting:
                    apMenuPane->SetItemDimmed(ECmdStart,   ETrue);
                    apMenuPane->SetItemDimmed(ECmdRestart, ETrue);
                    apMenuPane->SetItemDimmed(ECmdStop,    EFalse);
                    apMenuPane->SetItemDimmed(ECmdReset,   ETrue);
                    break;

                case Mws::EHttpdStopping:
                    apMenuPane->SetItemDimmed(ECmdStart,   ETrue);
                    apMenuPane->SetItemDimmed(ECmdRestart, ETrue);
                    apMenuPane->SetItemDimmed(ECmdStop,    ETrue);
                    apMenuPane->SetItemDimmed(ECmdReset,   ETrue);
                    break;
                    
                case Mws::EHttpdStopped:
                case Mws::EHttpdTerminated:
                    apMenuPane->SetItemDimmed(ECmdStart,   ETrue);
                    apMenuPane->SetItemDimmed(ECmdRestart, ETrue);
                    apMenuPane->SetItemDimmed(ECmdStop,    ETrue);
                    apMenuPane->SetItemDimmed(ECmdReset,   EFalse);
                }
            
            CWLAN
                &wlan = iDocument.WLAN();
            TBool
                wlanActive    = false,
                wlanActivated = false;

            TRAP_IGNORE(wlanActive    = wlan.ActiveL());
            TRAP_IGNORE(wlanActivated = wlan.ActivatedL());

            apMenuPane->SetItemDimmed(ECmdStartWLAN, wlanActive);
            apMenuPane->SetItemDimmed(ECmdCloseWLAN, !wlanActivated);
            apMenuPane->SetItemDimmed(ECmdAttachWLAN,  
                                      !wlanActive || wlanActivated);
            }
        else
            AppUi()->DynInitMenuPaneL(aMenuId, apMenuPane);
        }


    void CMainView::SetMenuL(TInt aResource)
        {
        CEikButtonGroupContainer
            *pCba = Cba();

        if (pCba)
            {
            pCba->SetCommandSetL(aResource);
            pCba->MakeVisible(ETrue);
            pCba->DrawDeferred();
            }
        }

}
