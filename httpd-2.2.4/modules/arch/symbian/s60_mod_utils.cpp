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

#include "s60_mod_utils.h"
#include <aknglobalconfirmationquery.h>
#include <aknglobalmsgquery.h> 


namespace Apache
{
    //
    // Confirmation
    //

    MConfirmationCallback::~MConfirmationCallback()
        {
        }



    CConfirmation::CConfirmation() :
        CActive(CActive::EPriorityStandard)
        {
        CActiveScheduler::Add(this);
        }


    CConfirmation::~CConfirmation()
        {
        delete ipTimer;
        delete ipQuery;
        }
    
        
    CConfirmation* CConfirmation::NewL()
        {
        CConfirmation
            *pThis = NewLC();
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }
    
    
    CConfirmation* CConfirmation::NewLC()
        {
        CConfirmation
            *pThis = new (ELeave) CConfirmation;
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL();
        
        return pThis;
        }
    
    
    void CConfirmation::ConstructL()
        {
        ipQuery = CAknGlobalConfirmationQuery::NewL();
        ipTimer = CPeriodic::NewL(CActive::EPriorityStandard);
        }
    
    
    void CConfirmation::ShowL(const TDesC&           aText,
                              TInt                   aSoftkeys,
                              TTimeIntervalSeconds   aTimeout,
                              CAknQueryDialog::TTone aTone,
                              TBool                  aDismissWithAnyKey,
                              MCallback*             apCallback)
        {
        if (ipCallback)
            User::Leave(KErrInUse);

        if (aTimeout.Int() != 0)
            {
            const TTimeIntervalMicroSeconds32
                timeout(aTimeout.Int() * 1000000);
            TCallBack
                cb(&CConfirmation::DoTimerExpiredL, this);
            
            ipTimer->Start(timeout, timeout, cb);
            }

        TInt
            animation = 0;
        
        iButton = 0;
        
        ipQuery->ShowConfirmationQueryL(iStatus,
                                        aText,
                                        aSoftkeys,
                                        animation,
                                        KNullDesC,
                                        0,
                                        0,
                                        aTone,
                                        aDismissWithAnyKey);

        ipCallback = apCallback;
        
        SetActive();
        }

    
    TInt CConfirmation::DoTimerExpiredL(TAny* apThis)
        {
        CConfirmation
            *pThis = static_cast<CConfirmation*>(apThis);
        
        return pThis->TimerExpiredL();
        }

    
    TInt CConfirmation::TimerExpiredL()
        {
        iButton = KTimedOut;
        
        DoCancel();
        
        return 0;
        }

            
    void CConfirmation::RunL()
        {
        ipTimer->Cancel();
        
        MCallback
            *pCallback = ipCallback;
        
        ipCallback = 0;

        if (iButton == 0)
            iButton = iStatus.Int();
 
        pCallback->ConfirmedL(this, iButton);
        }
    

    void CConfirmation::DoCancel()
        {
        ipTimer->Cancel();
        ipQuery->CancelConfirmationQuery();
        }



    CConfirmer::CConfirmer()
        {
        }
    

    CConfirmer::~CConfirmer()
        {
        delete ipConfirmation;
        delete ipScheduler;
        }


    CConfirmer* CConfirmer::NewL()
        {
        CConfirmer
            *pThis = NewLC();
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }
    

    CConfirmer* CConfirmer::NewLC()
        {
        CConfirmer
            *pThis = new (ELeave) CConfirmer;
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL();
        
        return pThis;
        }

    
    void CConfirmer::ConstructL()
        {
        ipScheduler    = new (ELeave) CActiveSchedulerWait;
        ipConfirmation = Apache::CConfirmation::NewL();
        }


    TInt CConfirmer::ConfirmL(const TDesC&           aText,
                              TInt                   aSoftkeys,
                              TTimeIntervalSeconds   aTimeout,
                              CAknQueryDialog::TTone aTone)
        {
        iButton = 0;
        
        ipConfirmation->ShowL(aText, 
                              aSoftkeys,
                              aTimeout,
                              CAknQueryDialog::EConfirmationTone,
                              EFalse,
                              this);
            
        ipScheduler->Start();
        
        return iButton;
        }
    
    
    void CConfirmer::ConfirmedL(CConfirmation*, TInt aButton)
        {
        iButton = aButton;

        ipScheduler->AsyncStop();
        }


    //
    // Message
    //

    MMessageCallback::~MMessageCallback()
        {
        }



    CMessage::CMessage() :
        CActive(CActive::EPriorityStandard)
        {
        CActiveScheduler::Add(this);
        }


    CMessage::~CMessage()
        {
        delete ipTimer;
        delete ipQuery;
        }
    
        
    CMessage* CMessage::NewL()
        {
        CMessage
            *pThis = NewLC();
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }
    
    
    CMessage* CMessage::NewLC()
        {
        CMessage
            *pThis = new (ELeave) CMessage;
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL();
        
        return pThis;
        }
    
    
    void CMessage::ConstructL()
        {
        ipQuery = CAknGlobalMsgQuery::NewL();
        ipTimer = CPeriodic::NewL(CActive::EPriorityStandard);
        }
    
    
    void CMessage::ShowL(const TDesC&           aHeader,
                         const TDesC&           aText,
                         TInt                   aSoftkeys,
                         TTimeIntervalSeconds   aTimeout,
                         CAknQueryDialog::TTone aTone,
                         MCallback*             apCallback)
        {
        if (ipCallback)
            User::Leave(KErrInUse);

        if (aTimeout.Int() != 0)
            {
            const TTimeIntervalMicroSeconds32
                timeout(aTimeout.Int() * 1000000);
            TCallBack
                cb(&CMessage::DoTimerExpiredL, this);
            
            ipTimer->Start(timeout, timeout, cb);
            }

        iButton = 0;
        
        ipQuery->ShowMsgQueryL(iStatus,
                               aText,
                               aSoftkeys,
                               aHeader,
                               KNullDesC,
                               0,
                               0,
                               aTone);
        
        ipCallback = apCallback;
        
        SetActive();
        }

    
    TInt CMessage::DoTimerExpiredL(TAny* apThis)
        {
        CMessage
            *pThis = static_cast<CMessage*>(apThis);
        
        return pThis->TimerExpiredL();
        }

    
    TInt CMessage::TimerExpiredL()
        {
        iButton = KTimedOut;
        
        DoCancel();
        
        return 0;
        }

            
    void CMessage::RunL()
        {
        ipTimer->Cancel();
        
        MCallback
            *pCallback = ipCallback;
        
        ipCallback = 0;
 
        if (iButton == 0)
            iButton = iStatus.Int();

        pCallback->AcknowledgedL(this, iButton);
        }
    

    void CMessage::DoCancel()
        {
        ipTimer->Cancel();
        ipQuery->CancelMsgQuery();
        }



    CMessenger::CMessenger()
        {
        }
    

    CMessenger::~CMessenger()
        {
        delete ipMessage;
        delete ipScheduler;
        }


    CMessenger* CMessenger::NewL()
        {
        CMessenger
            *pThis = NewLC();
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }
    

    CMessenger* CMessenger::NewLC()
        {
        CMessenger
            *pThis = new (ELeave) CMessenger;
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL();
        
        return pThis;
        }

    
    void CMessenger::ConstructL()
        {
        ipScheduler = new (ELeave) CActiveSchedulerWait;
        ipMessage   = Apache::CMessage::NewL();
        }


    TInt CMessenger::DeliverL(const TDesC&           aHeader,
                              const TDesC&           aText,
                              TInt                   aSoftkeys,
                              TTimeIntervalSeconds   aTimeout,
                              CAknQueryDialog::TTone aTone)
        {
        iButton = 0;
        
        ipMessage->ShowL(aHeader,
                         aText, 
                         aSoftkeys,
                         aTimeout,
                         aTone,
                         this);
            
        ipScheduler->Start();
        
        return iButton;
        }
    
    
    void CMessenger::AcknowledgedL(CMessage*, TInt aButton)
        {
        iButton = aButton;
        
        ipScheduler->AsyncStop();
        }
}
