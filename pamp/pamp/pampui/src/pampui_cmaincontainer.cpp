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

#include "pampui/cmaincontainer.h"
#include <aknutils.h>
#include <cdblen.h>
#include <eiklabel.h>


using namespace PampUi;


namespace
{
#if defined(_UNICODE)
#define TEXT(X) ((const TText*) L##X)
#else
#define TEXT(X) X
#endif
    

    const struct LabelData
        {
        const TText* ipText;
        } labels[] = 
            {
            { TEXT("Apache:") },
            { TEXT("MySQL:") },
            { TEXT("WLAN AP:") },
            { TEXT("WLAN IP:") },
            };
    
    const TInt KElementCount   = sizeof(labels)/sizeof(labels[0]);
    const TInt KGranularity    = KElementCount;
    
    const TInt KIndexHttpd  = 0;
    const TInt KIndexMySQL  = 1;
    const TInt KIndexWLANAP = 2;
    const TInt KIndexWLANIP = 3;
    
    const TInt KLabelWidth = 60;
    const TInt KTextWidth  = 105;
    const TInt KRowHeight  = 15;
    
    const TInt KLabelX = 5;
    const TInt KTextX  = 67;
    const TInt KOrigoY = 6;
    
    const TInt KMarginDx = 3;
    const TInt KMarginDy = 2;
}


namespace PampUi
{

    using namespace Mws;
    using namespace MySQL;


    CMainContainer::CMainContainer()
        {
        }


    CMainContainer::~CMainContainer()
        {
        ipLabels->ResetAndDestroy();
        ipTexts->ResetAndDestroy();
        
        delete ipLabels;
        delete ipTexts;

        delete ipApache;
        delete ipMySQL;
        delete ipPHP;
        }


    CMainContainer* CMainContainer::NewL(const TRect& aRect)
        {
        CMainContainer
            *pThis = NewLC(aRect);
        
        CleanupStack::Pop(pThis);

        return pThis;
        }


    CMainContainer* CMainContainer::NewLC(const TRect& aRect)
        {
        CMainContainer
            *pThis = new (ELeave) CMainContainer;
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL(aRect);
        return pThis;
        }

    
    namespace
        {
            CEikLabel* CreateLabelL(CCoeControl*  apContainer,
                                    const TDesC&  aText,
                                    const TPoint& aPos,
                                    const TSize&  aSize)
                {
                CEikLabel
                    *pLabel = new (ELeave) CEikLabel;
                
                pLabel->SetContainerWindowL(*apContainer);
                pLabel->SetTextL(aText);
                pLabel->SetExtent(aPos, aSize);
                pLabel->SetFont(LatinPlain12());
                
                return pLabel;
                }


            CEikLabel* CreateTextL(CCoeControl*  apContainer,
                                   const TPoint& aPos,
                                   const TSize&  aSize)
                {
                CEikLabel
                    *pLabel = new (ELeave) CEikLabel;
                
                pLabel->SetContainerWindowL(*apContainer);
                pLabel->SetTextL(_L(""));
                pLabel->SetExtent(aPos, aSize);
                pLabel->SetFont(LatinPlain12());
                
                return pLabel;
                }
        }
    
    

    namespace
        {
            void LoadBitmapsL(CFbsBitmap** appApache,
                              CFbsBitmap** appMySQL,
                              CFbsBitmap** appPHP)
                {
                TFileName
                    path(_L("\\resource\\apps\\pampui.mbm"));
                
                CompleteWithAppPath(path);
                
                CFbsBitmap
                    *pBitmap;
                
                // Apache
                pBitmap = new (ELeave) CFbsBitmap;
                
                //pBitmap->Load(path, EMbmPampuiApache_bo_248x24);
                pBitmap->Load(path, EMbmPampuiApache_pb_248x24);
                
                *appApache = pBitmap;

                // MySQL
                pBitmap = new (ELeave) CFbsBitmap;
                
                pBitmap->Load(path, EMbmPampuiPowered_by_mysql_125x64);
                
                *appMySQL = pBitmap;

                // PHP
                pBitmap = new (ELeave) CFbsBitmap;
                
                pBitmap->Load(path, EMbmPampuiPhp_power_white);
                
                *appPHP = pBitmap;
                }
        }



    void CMainContainer::ConstructL(const TRect& aRect)
        {
        ipLabels = new (ELeave) CArrayPtrFlat<CEikLabel>(KGranularity);
        ipTexts = new (ELeave) CArrayPtrFlat<CEikLabel>(KGranularity);

        CreateWindowL();
        
        const TSize
            KLabelSize = TSize(KLabelWidth, KRowHeight),
            KTextSize  = TSize(KTextWidth, KRowHeight);

        TInt
            i;

        for (i = 0; i < KElementCount; ++i)
            {
            TInt
                y = KOrigoY + i * KRowHeight;
            
            CEikLabel
                *pLabel = CreateLabelL(this, TPtrC(labels[i].ipText),
                                       TPoint(KLabelX, y), KLabelSize);
            
            ipLabels->AppendL(pLabel);
            
            CEikLabel
                *pText = CreateTextL(this,
                                     TPoint(KTextX, y), KTextSize);
            
            ipTexts->AppendL(pText);
            }

        LoadBitmapsL(&ipApache, &ipMySQL, &ipPHP);
        
        SetRect(aRect);
        ActivateL();
        }


    void CMainContainer::Draw(const TRect& aRect) const
        {
        CWindowGc
            &gc = SystemGc();
        
        gc.Clear(aRect);

        TInt
            x0 = KTextX - KMarginDx,
            x1 = x0 + KTextWidth + KMarginDx,
            y0 = KOrigoY - KMarginDy;
        TInt
            y = 0;
        
        for (TInt i = 0; i <= KElementCount; ++i)
            {
            y = y0 + i * KRowHeight;
            
            TPoint
                p0(x0, y),
                p1(x1, y);
            
            gc.DrawLine(p0, p1);
            }
        
        gc.DrawLine(TPoint(x0, y0), TPoint(x0, y));
        gc.DrawLine(TPoint(x1, y0), TPoint(x1, y));
        
        TInt
            x;
        TRect
            rect = Rect();
        
        TSize
            sizePHP   = ipPHP->SizeInPixels(),
            sizeMySQL = ipMySQL->SizeInPixels();

        TInt
            height = Max(sizePHP.iHeight, sizeMySQL.iHeight);
        
        // PHP
        
        x = rect.Width() / 2 + rect.Width() / 4 - sizePHP.iWidth / 2;
        y = rect.Height() - height / 2 - sizePHP.iHeight / 2;

        gc.BitBlt(TPoint(x, y), ipPHP);

        // MySQL
        
        x = rect.Width() / 4 - sizeMySQL.iWidth / 2;
        y = rect.Height() - height /  2 - sizeMySQL.iHeight / 2;

        gc.BitBlt(TPoint(x, y), ipMySQL);

        // Apache
        TSize
            sizeApache = ipApache->SizeInPixels();
        
        x = rect.Width() / 2 - sizeApache.iWidth / 2;
        y = rect.Height() - height - sizeApache.iHeight;

        gc.BitBlt(TPoint(x, y), ipApache);

        }
        
    
    TInt CMainContainer::CountComponentControls() const
        {
        return 2 * KElementCount;
        }
        
    
    CCoeControl* CMainContainer::ComponentControl(TInt aIndex) const
        {
        const CArrayPtr<CEikLabel>
            *pArray;
        
        if (aIndex < KElementCount)
            pArray = ipLabels;
        else
            {
            pArray = ipTexts;
            
            aIndex -= KElementCount;
            }
        
        return pArray->At(aIndex);
        }



    void CMainContainer::HandleControlEventL(CCoeControl* /*aControl*/,
                                             TCoeEvent    /*aEventType*/)
        {
        }


    void CMainContainer::UpdateStateL(CHttpd*        /*apHttpd*/, 
                                      CHttpd::TState aState)
        {
        CEikLabel
            *pText = ipTexts->At(KIndexHttpd);
        
        switch (aState)
            {
            case EHttpdNotStarted:
                pText->SetTextL(_L("Not Running"));
                break;
                
            case EHttpdStarting:
                pText->SetTextL(_L("Starting"));
                break;
                
            case EHttpdRunning:
                pText->SetTextL(_L("Running"));
                break;
                
            case EHttpdRestarting:
                pText->SetTextL(_L("Restarting"));
                break;
                
            case EHttpdStopping:
                pText->SetTextL(_L("Stopping"));
                break;

            case EHttpdStopped:
                pText->SetTextL(_L("Stopped"));
                break;

            case EHttpdTerminated:
                pText->SetTextL(_L("Terminated"));
                break;
            }
        
        pText->DrawDeferred();
        }


    // MySQL::CMysqld::MObserver
    void CMainContainer::UpdateStateL(CMysqld*        /* apMySQL */, 
                                      CMysqld::TState aState)
        {
        CEikLabel
            *pText = ipTexts->At(KIndexMySQL);
        
        switch (aState)
            {
            case EMysqldNotRunning:
                pText->SetTextL(_L("Not Running"));
                break;
                
            case EMysqldRunning:
                pText->SetTextL(_L("Running"));
                break;
                
            }
        
        pText->DrawDeferred();
        }


    void CMainContainer::UpdateL(CWLAN*                          apWLAN,
                                 const CWLAN::TConnectionStatus& aStatus)
        {
        TInt
            rc;

        TBuf<KCommsDbSvrDefaultTextFieldLength>
            ap;
        TBuf<39> // For this magic number see TInetAddr::Output()
            ip;

        if (aStatus.iProgress.iStage != KConnectionUninitialised)
            {
            TRAP(rc, apWLAN->GetIpAddressL(aStatus.iInfo, ip));
            
            if (rc == KErrNone)
                {
                TRAP(rc, apWLAN->GetApNameL(aStatus.iInfo.iIapId, ap));
                
                if (rc != KErrNone)
                    ap.Append(_L("Unknown"));
                }
            }
        
        CEikLabel
            *pAPLabel = ipTexts->At(KIndexWLANAP),
            *pIPLabel = ipTexts->At(KIndexWLANIP);

        pAPLabel->SetTextL(ap);
        pAPLabel->DrawDeferred();

        pIPLabel->SetTextL(ip);
        pIPLabel->DrawDeferred();
        }

} // namespace MwsUi
