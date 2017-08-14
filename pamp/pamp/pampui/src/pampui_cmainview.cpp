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

#include "pampui/cmainview.h"
#include <aknnotedialog.h>
#include <aknviewappui.h>
#include <avkon.rsg>
#include <bautils.h>
#include <eikbtgpc.h>
#include <eikmenup.h>
#include "mws/cargumentsquerydialog.h" // The one from MySQL would also be ok.
#include "pampui/cmaincontainer.h"
#include "pampui/cdocument.h"

using namespace Mws;
using namespace MySQL;


namespace
{
    _LIT(KHttpdPrompt, "Httpd arguments");
    _LIT(KMySQLPrompt, "MySQL arguments");

    
    HBufC* QueryArgumentsLC(const TDesC&                    aPrompt,  
                            const TDesC&                    aDefault,
                            TInt                            aMaxLength,
                            CArgumentsQueryDialog::TButtons aButtons)
        
        {
        TInt
            length = Max(aDefault.Length(), aMaxLength);
        HBufC
            *pArguments = HBufC::NewLC(length);
        
        *pArguments = aDefault;
        
        TPtr
            buffer = pArguments->Des();
        
        CArgumentsQueryDialog
            *pDialog = CArgumentsQueryDialog::NewL(aPrompt,
                                                   buffer,
                                                   aButtons);
        
        TInt
            rc = pDialog->RunLD();
        
        if ((rc != EAknSoftkeySave) && (rc != EAknSoftkeyOk))
            {
            CleanupStack::PopAndDestroy(pArguments);
            
            pArguments = 0;
            }
        
        return pArguments;
        }
}
    

namespace PampUi
{
    
    CMainView::CMainView(CDocument* apDocument) :
        iDocument(*apDocument)
        {
        }


    CMainView::~CMainView()
        {
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
        BaseConstructL(R_PPUI_MAIN_VIEW);
        }


    TUid CMainView::Id() const
        {
        return TUid::Uid(KId);
        }


    void CMainView::HandleCommandL(TInt aCommand)
        {
        switch (aCommand)
            {
            case ECmdStartPamp:
                StartHttpd(iDocument.HttpdArguments().Get());
                StartMysqld(iDocument.MysqldArguments().Get());

                ActivateWLANL();
                break;
                
            case ECmdStartHttpd:
                {
                HBufC
                    *pArguments = 
                    QueryArgumentsLC(KHttpdPrompt,
                                     iDocument.HttpdArguments().Get(),
                                     KHttpdMaxArgumentsLength,
                                     CArgumentsQueryDialog::EOkCancel);
                                              
                if (pArguments)
                    {
                    StartHttpd(*pArguments);
                    
                    CleanupStack::PopAndDestroy(pArguments);
                    }
                else
                    StartHttpd(iDocument.HttpdArguments().Get());

                ActivateWLANL();
                }
                break;
                
            case ECmdStartMysqld:
                {
                HBufC
                    *pArguments = 
                    QueryArgumentsLC(KMySQLPrompt,
                                     iDocument.MysqldArguments().Get(),
                                     KMysqldMaxArgumentsLength,
                                     CArgumentsQueryDialog::EOkCancel);
                
                if (pArguments)
                    {
                    StartMysqld(*pArguments);
                    
                    CleanupStack::PopAndDestroy(pArguments);
                    }
                else
                    StartMysqld(iDocument.MysqldArguments().Get());

                ActivateWLANL();
                }
                break;
                
            case ECmdStopPamp:
                HandleCommandL(ECmdStopHttpd);
                HandleCommandL(ECmdStopMysqld);
                break;
                
            case ECmdStopHttpd:
                {
                CHttpd
                    &httpd = iDocument.Httpd();

                httpd.StopL();
                }
                break;
                
            case ECmdStopMysqld:
                {
                CMysqld
                    &mysqld = iDocument.Mysqld();

                mysqld.StopL();
                }
                break;

            case ECmdStartWLAN:
            case ECmdAttachWLAN:
                iDocument.WLAN().ActivateL();
                break;

            case ECmdCloseWLAN:
                iDocument.WLAN().Close();
                break;

            case ECmdArgumentsHttpd:
                {
                HBufC
                    *pArguments = 
                    QueryArgumentsLC(KHttpdPrompt,
                                     iDocument.HttpdArguments().Get(),
                                     KHttpdMaxArgumentsLength,
                                     CArgumentsQueryDialog::ESaveCancel);
                                              
                if (pArguments)
                    {
                    iDocument.SetHttpdArgumentsL(*pArguments);

                    CleanupStack::PopAndDestroy(pArguments);
                    }
                }
                break;

            case ECmdArgumentsMysqld:
                {
                HBufC
                    *pArguments = 
                    QueryArgumentsLC(KMySQLPrompt,
                                     iDocument.MysqldArguments().Get(),
                                     KMysqldMaxArgumentsLength,
                                     CArgumentsQueryDialog::ESaveCancel);
                
                if (pArguments)
                    {
                    iDocument.SetMysqldArgumentsL(*pArguments);

                    CleanupStack::PopAndDestroy(pArguments);
                    }
                }
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
                                const TDesC8&     /* aCustomMessage */)
        {
        if (!ipContainer)
            {
            ipContainer = CMainContainer::NewL(ClientRect());
            ipContainer->SetMopParent(this);

            iDocument.Httpd().AttachL(ipContainer);
            iDocument.Mysqld().AttachL(ipContainer);
            
            iDocument.WLAN().AttachL(ipContainer);

            AppUi()->AddToStackL(*this, ipContainer);
            }
        }

    
    void CMainView::DoDeactivate()
        {
        if (ipContainer)
            {
            AppUi()->RemoveFromViewStack(*this, ipContainer);

            iDocument.WLAN().Detach(ipContainer);

            iDocument.Mysqld().Detach(ipContainer);
            iDocument.Httpd().Detach(ipContainer);

            delete ipContainer;
            ipContainer = 0;
            }
        }

    
    namespace
        {
            inline void EnableCmd(CEikMenuPane* apMenuPane,
                                  TInt          aItem,
                                  TBool         aEnable)
                {
                apMenuPane->SetItemDimmed(aItem, !aEnable);
                }
        }


    namespace
        {
            void GetStates(const CHttpd&  aHttpd,
                           THttpdState&   aHttpdState,
                           const CMysqld& aMysqld, 
                           TMysqldState&  aMysqldState)
                {
                aHttpdState  = aHttpd.StateL();
                aMysqldState = aMysqld.StateL();
                }
        }


    void CMainView::DynInitMenuPaneL(TInt aMenuId, CEikMenuPane* apMenuPane)
        {
        TBool
            startPamp   = EFalse,
            startHttpd  = EFalse,
            startMysqld = EFalse,
            stopPamp    = EFalse,
            stopHttpd   = EFalse,
            stopMysqld  = EFalse;
        
        THttpdState
            httpdState = EHttpdNotStarted;
        TMysqldState
            mysqlState = EMysqldNotRunning;
        
        TRAPD(error, GetStates(iDocument.Httpd(), httpdState, 
                               iDocument.Mysqld(), mysqlState));

        if (error == KErrNone)
            {
            startHttpd  = (httpdState == EHttpdNotStarted);
            startMysqld = (mysqlState == EMysqldNotRunning);
            startPamp   = startHttpd && startMysqld;
            stopHttpd   = (httpdState != EHttpdNotStarted) &&
                          (httpdState != EHttpdStopped) &&
                          (httpdState != EHttpdTerminated);
            stopMysqld  = (mysqlState == EMysqldRunning);
            stopPamp    = stopHttpd && stopMysqld;
            }
            

        switch (aMenuId)
            {
            case R_PPUI_MAIN_MENU:
                {
                CWLAN
                    &wlan = iDocument.WLAN();
                TBool
                    wlanActive   = false,
                    wlanAttached = false;
                
                TRAP_IGNORE(wlanActive   = wlan.ActiveL());
                TRAP_IGNORE(wlanAttached = wlan.ActivatedL());
                
                apMenuPane->SetItemDimmed(ECmdStartWLAN, wlanActive);
                apMenuPane->SetItemDimmed(ECmdCloseWLAN, !wlanAttached);
                apMenuPane->SetItemDimmed(ECmdAttachWLAN, 
                                          !wlanActive || wlanAttached);
                
                EnableCmd(apMenuPane, ECmdStart, startHttpd || startMysqld);
                EnableCmd(apMenuPane, ECmdStop,  stopHttpd || stopMysqld);
                }
                break;

            case R_PPUI_START_MENU:
                EnableCmd(apMenuPane, ECmdStartPamp,   startPamp);
                EnableCmd(apMenuPane, ECmdStartHttpd,  startHttpd);
                EnableCmd(apMenuPane, ECmdStartMysqld, startMysqld);
                break;
                
            case R_PPUI_STOP_MENU:            
                EnableCmd(apMenuPane, ECmdStopPamp,   stopPamp);
                EnableCmd(apMenuPane, ECmdStopHttpd,  stopHttpd);
                EnableCmd(apMenuPane, ECmdStopMysqld, stopMysqld);                
                break;
                
            case R_PPUI_SETTINGS_MENU:
                break;

            default:
                AppUi()->DynInitMenuPaneL(aMenuId, apMenuPane);
            }
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


    void CMainView::ActivateWLANL()
        {
        CWLAN
            &wlan = iDocument.WLAN();
        
        if (!wlan.ActivatedL())
            {
            TInt
                id;
            
            if (wlan.ActiveL())
                id = R_PPUI_ATTACH_WLAN;
            else
                id = R_PPUI_START_WLAN;
            
            CAknNoteDialog
                *pDialog = new (ELeave) CAknNoteDialog;
            
            if (pDialog->ExecuteLD(id))
                wlan.ActivateL();
            }
        }


    void CMainView::StartHttpd(const TDesC& aArguments)
        {
        CHttpd
            &httpd = iDocument.Httpd();
        
        httpd.StartL(aArguments, iDocument.HttpdSettings());
        }


    void CMainView::StartMysqld(const TDesC& aArguments)
        {
        CMysqld
            &mysqld = iDocument.Mysqld();
        
        mysqld.StartL(aArguments);
        }

} // PampUi
