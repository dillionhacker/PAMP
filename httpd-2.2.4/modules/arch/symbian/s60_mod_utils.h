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

// Symbian
#include <e32base.h>
#include <txtrich.h>
#include "symbian_mod_utils.h"

// S60
#include <aknquerydialog.h>

// Apache
#include "httpd.h"
#include "http_protocol.h"


class CAknGlobalConfirmationQuery;
class CAknGlobalMsgQuery;

namespace Apache
{
    //
    // Confirmation
    //

    class CConfirmation;
    
    
    NONSHARABLE_CLASS(MConfirmationCallback)
        {
    public:
        virtual ~MConfirmationCallback();

        virtual void ConfirmedL(CConfirmation*, TInt aButton) = 0;
        };


    NONSHARABLE_CLASS(CConfirmation) : public CActive
        {
    public:
        typedef MConfirmationCallback MCallback;

        enum
        {
        KInfinite = 0,
        KTimedOut = KMinTInt
        };

        ~CConfirmation();

        static CConfirmation* NewL();
        static CConfirmation* NewLC();
 
        void ShowL(const TDesC&           aText,
                   TInt                   aSoftkeys,
                   TTimeIntervalSeconds   aTimeout,
                   CAknQueryDialog::TTone aTone,
                   TBool                  aDismissWithAnyKey,
                   MCallback*             apCallback);

    private:
        CConfirmation();

        void ConstructL();

    private:
        static TInt DoTimerExpiredL(TAny* apThis);
        
        TInt TimerExpiredL();

    private:
        // CActive
        void RunL();

        void DoCancel();

    private:
        CAknGlobalConfirmationQuery* ipQuery;
        CPeriodic*                   ipTimer;
        TInt                         iButton;
        MCallback*                   ipCallback;
        };




    NONSHARABLE_CLASS(CConfirmer) : public  CBase,
                                    private CConfirmation::MCallback
        {
    public:
        ~CConfirmer();

        static CConfirmer* NewL();
        static CConfirmer* NewLC();
        
        TInt ConfirmL(const TDesC&         aText,
                      TInt                 aSoftkeys,
                      TTimeIntervalSeconds aTimeout = CConfirmation::KInfinite,
                      CAknQueryDialog::TTone aTome = CAknQueryDialog::ENoTone);

    private:
        CConfirmer();

        void ConstructL();

    private:
        void ConfirmedL(Apache::CConfirmation*, TInt aButton);

    private:
        CActiveSchedulerWait*  ipScheduler;
        Apache::CConfirmation* ipConfirmation;
        TInt                   iButton;
        };


    //
    // Message
    //

    class CMessage;
    
    
    NONSHARABLE_CLASS(MMessageCallback)
        {
    public:
        virtual ~MMessageCallback();

        virtual void AcknowledgedL(CMessage*, TInt aButton) = 0;
        };


    NONSHARABLE_CLASS(CMessage) : public CActive
        {
    public:
        typedef MMessageCallback MCallback;

        enum
        {
        KInfinite = 0,
        KTimedOut = KMinTInt
        };

        ~CMessage();

        static CMessage* NewL();
        static CMessage* NewLC();
 
        void ShowL(const TDesC&           aHeader,
                   const TDesC&           aText,
                   TInt                   aSoftkeys,
                   TTimeIntervalSeconds   aTimeout,
                   CAknQueryDialog::TTone aTone,
                   MCallback*             apCallback);

    private:
        CMessage();

        void ConstructL();

    private:
        static TInt DoTimerExpiredL(TAny* apThis);
        
        TInt TimerExpiredL();

    private:
        // CActive
        void RunL();

        void DoCancel();

    private:
        CAknGlobalMsgQuery* ipQuery;
        CPeriodic*          ipTimer;
        TInt                iButton;
        MCallback*          ipCallback;
        };




    NONSHARABLE_CLASS(CMessenger) : public  CBase,
                                    private CMessage::MCallback
        {
    public:
        ~CMessenger();
        
        static CMessenger* NewL();
        static CMessenger* NewLC();
        
        TInt DeliverL(const TDesC&           aHeader,
                      const TDesC&           aText,
                      TInt                   aSoftkeys,
                      TTimeIntervalSeconds   aTimeout = CMessage::KInfinite,
                      CAknQueryDialog::TTone aTone = CAknQueryDialog::ENoTone);
        
    private:
        CMessenger();

        void ConstructL();

    private:
        void AcknowledgedL(Apache::CMessage*, TInt aButton);

    private:
        CActiveSchedulerWait* ipScheduler;
        Apache::CMessage*     ipMessage;
        TInt                  iButton;
        };
}
