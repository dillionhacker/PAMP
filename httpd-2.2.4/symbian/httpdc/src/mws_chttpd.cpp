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

#include "mws/chttpd.h"


namespace
{
    enum
        {
        KGranularity = 8
        };
}


namespace
{
    using namespace Mws;

    class MStateObserver
        {
    public:
        virtual void UpdateState(CHttpd::TState aState) = 0;
        };
    

    class CStateSubject : public ::CActive
        {
    public:
        static CStateSubject* NewL(MStateObserver* apObserver, 
                                   RHttpd*         apHttpd)
            {
            CStateSubject
                *pThis = NewLC(apObserver, apHttpd);
            
            CleanupStack::Pop(pThis);

            return pThis;
            }


        static CStateSubject* NewLC(MStateObserver* apObserver, 
                                    RHttpd*         apHttpd)
            {
            CStateSubject
                *pThis = new (ELeave) CStateSubject(apObserver, apHttpd);
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();

            return pThis;
            }

        
        void Start()
            {
            DoStart();
            SetActive();
            }
        
        void Stop()
            {
            Cancel();
            }

        
    private:
        void RunL()
            {
            switch (iStatus.Int())
                {
                case KErrNone:
                    if (DoUpdate())
                        Start();
                    break;
                    
                case KErrCancel:
                    break;

                case KErrServerTerminated:
                    iState = EHttpdTerminated;
                    DoUpdate();
                    break;
                    
                default:
                    User::LeaveIfError(iStatus.Int());
                }
            }

        
        void DoCancel()
            {
            iHttpd.GetStateCancel();
            }
        
        
        void DoStart()
            {
            iHttpd.GetState(&iPckgState, iStatus);
            }
        
        
        TBool DoUpdate()
            {
            iObserver.UpdateState(iState);
            
            return (iState != EHttpdStopped) && (iState != EHttpdTerminated);
            }

        
    private:
        CStateSubject(MStateObserver* apObserver, RHttpd* apHttpd) :
            CActive(EPriorityStandard),
            iObserver(*apObserver),
            iHttpd(*apHttpd),
            iPckgState(iState)
            {
            CActiveScheduler::Add(this);
            }

        void ConstructL()
            {
            }

    protected:
        MStateObserver&    iObserver;
        RHttpd&            iHttpd;
        THttpdState        iState;
        TPckg<THttpdState> iPckgState;
        };
}


namespace Mws
{
    namespace
        {
            TInt FindObserver(CArrayPtr<CHttpd::MObserver>& aObservers,
                              CHttpd::MObserver*            apObserver)
                {
                TInt
                    i = -1;
                
                for (TInt j = 0; j < aObservers.Count(); ++j)
                    {
                    if (aObservers.At(j) == apObserver)
                        {
                        i = j;
                        break;
                        }
                        }
                
                return i;
                }
        }


    class CHttpd::CImp 
        : public  ::CBase,
          private MStateObserver
        {
    public:
        static CImp* NewL(CHttpd* apOwner, TBool aAutoConnect)
            {
            CImp
                *pThis = NewLC(apOwner, aAutoConnect);
            
            CleanupStack::Pop(pThis);
            
            return pThis;
            }


        static CImp* NewLC(CHttpd* apOwner, TBool aAutoConnect)
            {
            CImp
                *pThis = new (ELeave) CImp(apOwner, aAutoConnect);
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();
            
            return pThis;
            }
    

        ~CImp()
            {
            if (ipStateSubject) 
                {
                ipStateSubject->Stop();
                delete ipStateSubject;
                }
            
            delete ipObservers;
            
            iHttpd.Close();
            }
        
        
        TVersion Version() const
            {
            return iHttpd.Version();
            }


        void ConnectL()
            {
            if (Connected())
                User::Leave(KErrInUse);
            
            User::LeaveIfError(iHttpd.Connect());
            
            if (ipObservers->Count() > 0)
                ipStateSubject->Start();
            
            THttpdState
                state = EHttpdTerminated;
            
            TRAPD(rc, state = StateL());
            
            if (rc == KErrNone)
                UpdateState(state);
            }

    
        void Close()
            {
            if (Connected())
                {
                ipStateSubject->Stop();
                iHttpd.Close();
                }
            }
        
        
        void ReConnectL()
            {
            Close();
            ConnectL();
            }
        

        TBool Connected() const
            {
            return iHttpd.Handle() != 0;
            }
        

        void StartL(const THttpdSettings& aSettings) const
            {
            VerifyConnectionL();
            
            User::LeaveIfError(iHttpd.StartServer(aSettings));
            }
        
        
        void StartL(const TDesC&          aCommandLine,
                    const THttpdSettings& aSettings) const
            {
            VerifyConnectionL();
            
            User::LeaveIfError(iHttpd.StartServer(aCommandLine, aSettings));
            }
        
        
        void StopL(TBool aGraceful) const
            {
            VerifyConnectionL();
            
            User::LeaveIfError(iHttpd.StopServer(aGraceful));
            }
        
        
        void RestartL(TBool aGraceful) const
            {
            VerifyConnectionL();
            
            User::LeaveIfError(iHttpd.RestartServer(aGraceful));
            }
        
        
        THttpdState StateL() const
            {
            VerifyConnectionL();
            
            THttpdState
                state;
            TInt
                rc = iHttpd.GetState(state);
            
            switch (rc)
                {
                case KErrNone:
                    break;
                    
                case KErrServerTerminated:
                    state = EHttpdTerminated;
                    break;
                    
                default:
                    User::Leave(rc);
                }
            
            return state;
            }
        
    
        void AttachL(MObserver* apObserver)
            {
            ipObservers->AppendL(apObserver);
            
            if ((ipObservers->Count() == 1) && Connected())
                ipStateSubject->Start();
            
            TRAP_IGNORE(apObserver->UpdateStateL(ipOwner, StateL()));
            }


        void Detach(MObserver* apObserver)
            {
            TInt
                i = FindObserver(*ipObservers, apObserver);
            
            __ASSERT_DEBUG(i != -1, User::Invariant());
            
            if (i != -1)
                {
                ipObservers->Delete(i);
                
                if ((ipObservers->Count() == 0) && Connected())
                    ipStateSubject->Stop();
                }
            }
        
        
        void SetSettingsL(const THttpdSettings& aSettings) const
            {
            VerifyConnectionL();

            User::LeaveIfError(iHttpd.SetSettings(aSettings));
            }


        THttpdSettings SettingsL() const
            {
            THttpdSettings
                settings;

            User::LeaveIfError(iHttpd.GetSettings(settings));

            return settings;
            }


        THttpdSettings CurrentSettingsL() const
            {
            THttpdSettings
                settings;

            User::LeaveIfError(iHttpd.GetCurrentSettings(settings));

            return settings;
            }


        void UpdateState(CHttpd::TState aState)
            {
            for (TInt i = 0; i < ipObservers->Count(); ++i)
                {
                MObserver
                    *pObserver = ipObservers->At(i);
                
                TRAP_IGNORE(pObserver->UpdateStateL(ipOwner, aState));
                }

            if ((aState == EHttpdStopped) || (aState == EHttpdTerminated))
                Close();
            }
        
        
    private:
        CImp(CHttpd* apOwner, TBool aAutoConnect)
            : ipOwner(apOwner),
              iAutoConnect(aAutoConnect)
            {
            }
        
        
        void ConstructL()
            {
            ipStateSubject = CStateSubject::NewL(this, &iHttpd);
            
            ipObservers = new CArrayPtrSeg<MObserver>(KGranularity);
            
            if (iAutoConnect)
                User::LeaveIfError(iHttpd.Connect());
            }


        void VerifyConnectionL() const
            {
            if (!Connected())
                {
                if (iAutoConnect)
                    const_cast<CImp*>(this)->ConnectL();
                else
                    User::Leave(KErrNotReady);
                }
            }

    private:
        CHttpd*               ipOwner;
        RHttpd                iHttpd;
        CStateSubject*        ipStateSubject;
        CArrayPtr<MObserver>* ipObservers;
        TBool                 iAutoConnect;
        };

}



namespace Mws
{
    CHttpd::CHttpd()
        {
        }


    EXPORT_C CHttpd::~CHttpd()
        {
        delete ipImp;
        }


    EXPORT_C CHttpd* CHttpd::NewL(TBool aAutoConnect)
        {
        CHttpd
            *pThis = NewLC(aAutoConnect);

        CleanupStack::Pop(pThis);

        return pThis;
        }


    EXPORT_C CHttpd* CHttpd::NewLC(TBool aAutoConnect)
        {
        CHttpd
            *pThis = new (ELeave) CHttpd;
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL(aAutoConnect);
        
        return pThis;
        }
    

    void CHttpd::ConstructL(TBool aAutoConnect)
        {
        ipImp = CImp::NewL(this, aAutoConnect);
        }
    
    
    EXPORT_C TVersion CHttpd::Version() const
        {
        return ipImp->Version();
        }
    

    EXPORT_C void CHttpd::ConnectL()
        {
        return ipImp->ConnectL();
        }

    
    EXPORT_C void CHttpd::Close()
        {
        return ipImp->Close();
        }


    EXPORT_C void CHttpd::ReConnectL()
        {
        return ipImp->ReConnectL();
        }


    EXPORT_C TBool CHttpd::Connected() const
        {
        return ipImp->Connected();
        }


    EXPORT_C void CHttpd::StartL(const THttpdSettings& aSettings) const
        {
        return ipImp->StartL(aSettings);
        }


    EXPORT_C void CHttpd::StartL(const TDesC&          aCommandLine,
                                 const THttpdSettings& aSettings) const
        {
        return ipImp->StartL(aCommandLine, aSettings);
        }


    EXPORT_C void CHttpd::StopL(TBool aGraceful) const
        {
        return ipImp->StopL(aGraceful);
        }
    
    
    EXPORT_C void CHttpd::RestartL(TBool aGraceful) const
        {
        return ipImp->RestartL(aGraceful);
        }
    
    
    EXPORT_C THttpdState CHttpd::StateL() const
        {
        return ipImp->StateL();
        }
    
    
    EXPORT_C void CHttpd::AttachL(MObserver* apObserver)
        {
        return ipImp->AttachL(apObserver);
        }


    EXPORT_C void CHttpd::Detach(MObserver* apObserver)
        {
        return ipImp->Detach(apObserver);
        }


    EXPORT_C void CHttpd::SetSettingsL(const TSettings& aSettings) const
        {
        ipImp->SetSettingsL(aSettings);
        }


    EXPORT_C THttpdSettings CHttpd::SettingsL() const
        {
        return ipImp->SettingsL();
        }


    EXPORT_C THttpdSettings CHttpd::CurrentSettingsL() const
        {
        return ipImp->CurrentSettingsL();
        }        

}
