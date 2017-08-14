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

#include "httpdui/cmaincontainer.h"
#include <aknutils.h>
#include <cdblen.h>
#include <commdb.h>
#include <eiklabel.h>
#include <in_sock.h>
#include "cwlan.h"

using namespace Mws;


namespace
{

#if defined(_UNICODE)
#define TEXT(X) ((const TText*) L##X)
#else
#define TEXT(X) X
#endif


    const struct LabelData
        {
        const TText* iText;
        } labels[] = 
            {
            { TEXT("State:") },
            { TEXT("WLAN AP:") },
            { TEXT("WLAN IP:") }
            };
    
    const TInt KElementCount = sizeof(labels)/sizeof(labels[0]);
    const TInt KGranularity = KElementCount;

    const TInt KIndexState  = 0;
    const TInt KIndexWLANAP = 1;
    const TInt KIndexWLANIP = 2;

    const TInt KLabelWidth = 60;
    const TInt KTextWidth  = 105;
    const TInt KRowHeight  = 15;
    
    const TInt KLabelX = 5;
    const TInt KTextX  = 67;
    const TInt KOrigoY = 6;
    
    const TInt KMarginDx = 3;
    const TInt KMarginDy = 2;
}


namespace HttpdUi
{

    CMainContainer::CMainContainer()
        {
        }


    CMainContainer::~CMainContainer()
        {
        ipLabels->ResetAndDestroy();
        ipTexts->ResetAndDestroy();
        
        delete ipLabels;
        delete ipTexts;

        delete ipBitmap;
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
    
    
    void CMainContainer::ConstructL(const TRect& aRect)
        {
        ipLabels = new CArrayPtrFlat<CEikLabel>(KGranularity);
        ipTexts = new CArrayPtrFlat<CEikLabel>(KGranularity);
        
        const TSize
            KLabelSize = TSize(KLabelWidth, KRowHeight),
            KTextSize  = TSize(KTextWidth, KRowHeight);
                                      
        CreateWindowL();

        TInt
            i;
        
        for (i = 0; i < KElementCount; ++i)
            {
            TInt
                y = KOrigoY + i * KRowHeight;
            
            CEikLabel
                *pLabel = CreateLabelL(this, TPtrC(labels[i].iText),
                                       TPoint(KLabelX, y), KLabelSize);
            
            ipLabels->AppendL(pLabel);
            
            CEikLabel
                *pText = CreateTextL(this,
                                     TPoint(KTextX, y), KTextSize);
            
            ipTexts->AppendL(pText);
            }

        ipBitmap = new (ELeave) CFbsBitmap;

        TFileName
            path(Mws::KHttpdUtilMBMPath);
        
        CompleteWithAppPath(path);

        TInt
            id;

        if (aRect.Width() >= Mws::KApachePbWideWidth)
            id = EMbmHttpdutilApache_pb_248x24;
        else
            id = EMbmHttpdutilApache_pb_176x17;
        
        ipBitmap->Load(path, id);

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

        TRect
            rect = Rect();
        TSize
            size = ipBitmap->SizeInPixels();
        TInt
            x;
        
        x = rect.Width() / 2 - size.iWidth / 2;
        y = rect.Height() - size.iHeight;

        gc.BitBlt(TPoint(x, y), ipBitmap);
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


    void CMainContainer::UpdateStateL(CHttpd*   /*apHttpd*/, 
                                            THttpdState aState)
        {
        CEikLabel
            *pLabel = ipTexts->At(KIndexState);
        
        switch (aState)
            {
            case EHttpdNotStarted:
                pLabel->SetTextL(_L("Not Started"));
                break;
                
            case EHttpdStarting:
                pLabel->SetTextL(_L("Starting"));
                break;
                
            case EHttpdRunning:
                pLabel->SetTextL(_L("Running"));
                break;
                
            case EHttpdRestarting:
                pLabel->SetTextL(_L("Restarting"));
                break;
                
            case EHttpdStopping:
                pLabel->SetTextL(_L("Stopping"));
                break;

            case EHttpdStopped:
                pLabel->SetTextL(_L("Stopped"));
                break;

            case EHttpdTerminated:
                pLabel->SetTextL(_L("Terminated"));
                break;
            }
        
        pLabel->DrawDeferred();
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

}
