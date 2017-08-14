/* Copyright 2008 Nokia Corporation
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

/*
 * NOTE: This is general purpose stuff and is used by the UIs of MySQL,
 * NOTE: httpd, PAMP and Raccoon. As such it should be in some common DLL.
 * NOTE: However, currently there is no appropriate DLL, so for the
 * NOTE: time being it is simply copied around. A modification made
 * NOTE: in one place should be duplicated elsewhere.
 */

#include "cwlan.h"
#include <commdbconnpref.h>
#include <commdb.h>
#include <aputils.h>
#include <in_sock.h>


namespace
{
    class CInterfaceMonitor;

    NONSHARABLE_CLASS(MInterfaceMonitorOwner)
        {
    public:
        virtual void ErrorL(CInterfaceMonitor* apMonitor, TInt aError) = 0;
        };


    NONSHARABLE_CLASS(MInterfaceMonitorObserver)
        {
    public:
        virtual void UpdateL(CInterfaceMonitor* apMonitor,
                             const TInterfaceNotification& aNotification) = 0;
        };

    
    NONSHARABLE_CLASS(CInterfaceMonitor) : private CActive
        {
    public:
        typedef MInterfaceMonitorOwner    MOwner;
        typedef MInterfaceMonitorObserver MObserver;
        
        ~CInterfaceMonitor()
            {
            Stop();
            
            iConnection.Close();
            
            iObservers.Close();
            }
        

        static CInterfaceMonitor* NewL (RSocketServ* apSocketServer,
                                        MOwner*      apOwner)
            {
            CInterfaceMonitor
                *pThis = NewLC(apSocketServer, apOwner);
            
            CleanupStack::Pop(pThis);
            
            return pThis;
            }


        static CInterfaceMonitor* NewLC(RSocketServ* apSocketServer,
                                        MOwner*      apOwner)
            {
            CInterfaceMonitor
                *pThis = new (ELeave) CInterfaceMonitor(apSocketServer, 
                                                        apOwner);

            CleanupStack::PushL(pThis);
            pThis->ConstructL();
            
            return pThis;
            }
        

        void AttachL(MObserver* apObserver)
            {
            TInt
                count = iObservers.Count();
            
            iObservers.ReserveL(count + 1);
            
            if (count == 0) 
                {
                // First attachment. Time to start monitoring.
                Start();
                }
            
            // Will not leave due to ReserveL() above.
            iObservers.AppendL(apObserver); 
            }
        

        void Detach(MObserver* apObserver)
            {
            TInt
                i = iObservers.Count() - 1;
            
            while (i >= 0)
                {
                if (iObservers[i] == apObserver)
                    break;
                else
                    --i;
                }
            
            if (i >= 0)
                {
                iObservers.Remove(i);
                
                if (iObservers.Count() == 0)
                    // Last observer left; we can stop monitoring.
                    Stop();
                }
            }
        
      private:
        void DoCancel()
            {
            if (IsActive())
                iConnection.CancelAllInterfaceNotification();
            }
        
        
        void RunL()
            {
            if (iStatus.Int() == KErrNone)
                {
                Notify(iNotification);
                
                if (iObservers.Count() != 0)
                    Start();
                }
            else
                iOwner.ErrorL(this, iStatus.Int());
            }
        
      private:
        CInterfaceMonitor(RSocketServ* apSocketServer,
                          MOwner*      apOwner) :
            CActive(EPriorityStandard),
            iSocketServer(*apSocketServer),
            iOwner(*apOwner),
            iNotificationPackage(iNotification)
            {
            CActiveScheduler::Add(this);
            }
        

        void ConstructL()
            {
            User::LeaveIfError(iConnection.Open(iSocketServer));
            }


        void Notify(const TInterfaceNotification& aData)
            {
            // Count() is called during the loop by design. This way things 
            // will not break even if observers are attached/detached during 
            // the iteration. The result will be unpredictable, but there'll 
            // be no crash.
            
            for (TInt i = 0; i < iObservers.Count(); ++i)
                {
                MObserver
                    *pObserver = iObservers[i];
                
                TRAP_IGNORE(pObserver->UpdateL(this, aData));
                }
            }

            
        void Start()
            {
            iConnection.AllInterfaceNotification(iNotificationPackage, 
                                                 iStatus);
            SetActive();
            }
        
        
        void Stop()
            {
            if (IsActive())
                Cancel();
            }
        
      private:
        RSocketServ&                  iSocketServer;
        MOwner&                       iOwner;                      
        RConnection                   iConnection;
        TInterfaceNotification        iNotification;
        TPckg<TInterfaceNotification> iNotificationPackage;
        RPointerArray<MObserver>      iObservers;
        };
}


namespace
{
    class CConnectionMonitor;
    
    NONSHARABLE_CLASS(MConnectionMonitorObserver)
        {
    public:
        virtual void UpdateL(CConnectionMonitor* apMonitor,
                             const TNifProgress& aNotification) = 0;
        };


    NONSHARABLE_CLASS(CConnectionMonitor) : private ::CActive
        {
    public:
        typedef MConnectionMonitorObserver MObserver;
        

        ~CConnectionMonitor()
            {
            Stop();
            
            iConnection.Close();
            
            iObservers.Close();
            }
        

        static CConnectionMonitor* NewL(RSocketServ* apSocketServer)
            {
            CConnectionMonitor
                *pThis = NewLC(apSocketServer);
        
            CleanupStack::Pop(pThis);
            
            return pThis;
            }
        

        static CConnectionMonitor* NewLC(RSocketServ* apSocketServer)
            {
            CConnectionMonitor
                *pThis = new (ELeave) CConnectionMonitor(apSocketServer);
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();
            
            return pThis;
            }

     
        void AttachL(MObserver* apObserver)
            {
            iObservers.AppendL(apObserver);
            }
        
        
        void Detach(MObserver* apObserver)
            {
            TInt
                i = iObservers.Count() - 1;
            
            while (i >= 0)
                {
                if (iObservers[i] == apObserver)
                    break;
                else
                    --i;
                }
            
            if (i >= 0)
                iObservers.Remove(i);
            }

        
        void StartL(const TConnectionInfo& aInfo)
            {
            if (IsActive())
                User::Leave(KErrInUse);
            
            User::LeaveIfError(iConnection.Open(iSocketServer));
            
            TInt
                rc = iConnection.Attach(TPckg<TConnectionInfo>(aInfo), 
                                        RConnection::EAttachTypeMonitor);
            
            User::LeaveIfError(rc);
            
            iInfo = aInfo;
            iMonitoring = true;
            
            StartMonitoring();
            }
        
        
        void Stop()
            {
            if (IsActive())
                Cancel();
            
            iConnection.Close();
            
            iMonitoring = false;
            }
        
        
        TBool Monitoring() const
            {
            return iMonitoring;
            }


        TBool Monitoring(TConnectionInfo& aInfo) const
        {
        if (iMonitoring)
            aInfo = iInfo;

        return iMonitoring;
        }
        
    private:
        void DoCancel()
            {
            iConnection.CancelProgressNotification();
            }

        
        void RunL()
            {
            Notify(iProgress());
            
            if (iProgress().iStage != KConnectionUninitialised)
                {
                StartMonitoring();
                }
            else
                {
                Stop();
                }
            }

        
    private:
        CConnectionMonitor(RSocketServ* apSocketServer) 
            : CActive(EPriorityStandard),
            iSocketServer(*apSocketServer)
            {
            CActiveScheduler::Add(this);
            }
        

        void ConstructL()
            {
            }
        
        
        void Notify(const TNifProgress& aData)
            {
            // Count() is called during the loop by design. This way things 
            // will not break even if observers are attached/detached during 
            // the iteration. The result will be unpredictable, but there'll 
            // be no crash.
            
            for (TInt i = 0; i < iObservers.Count(); ++i)
                {
                MObserver
                    *pObserver = iObservers[i];
                
                TRAP_IGNORE(pObserver->UpdateL(this, aData));
                }
            }
        
        void StartMonitoring()
            {
            iConnection.ProgressNotification(iProgress, iStatus);
            SetActive();
            }
        
    private:
        RSocketServ&             iSocketServer;
        RConnection              iConnection;
        TNifProgressBuf          iProgress;
        TConnectionInfo          iInfo;
        TBool                    iMonitoring;
        RPointerArray<MObserver> iObservers;
        };
}


namespace
{
    TBool WLANActiveL(RConnection&     aConnection,
                      TConnectionInfo& aInfo)
        {
        TBool
            wlanActive = false;
        CCommsDatabase
            *pDb = CCommsDatabase::NewL();
                
        CleanupStack::PushL(pDb);
        
        CApUtils
            *pApUtils = CApUtils::NewLC(*pDb);
        
        TUint
            count = 0;
        
        User::LeaveIfError(aConnection.EnumerateConnections(count));
        
        for (TUint i = 1; (i <= count) && !wlanActive; ++i) 
            {
            TConnectionInfo
                info;
            TPckg<TConnectionInfo>
                package(info);
            TInt
                rc = aConnection.GetConnectionInfo(i, package);
            
            if (rc == KErrNone)
                {
                if (pApUtils->BearerTypeL(info.iIapId) == EApBearerTypeWLAN)
                    {
                    aInfo = info;
                    wlanActive = true;
                    }
                }
            }
        
        CleanupStack::PopAndDestroy(pApUtils);
        CleanupStack::PopAndDestroy(pDb);
        
        return wlanActive;
        }
    
    
    TBool WLANActiveL(RSocketServ&     aSocketServer,
                      TConnectionInfo& aInfo)
        {
        TBool
            wlanActive = false;
        
        RConnection
            connection;
        
        User::LeaveIfError(connection.Open(aSocketServer));
        CleanupClosePushL(connection);
        
        wlanActive = WLANActiveL(connection, aInfo);
        
        CleanupStack::PopAndDestroy(); // connection
        
        return wlanActive;
        }    
}


//
// CWLAN::CImp
//

NONSHARABLE_CLASS(CWLAN::CImp) : public ::CBase,
                                 private MConnectionMonitorObserver,
                                 private MInterfaceMonitorObserver,
                                 private MInterfaceMonitorOwner

    {
public:
    ~CImp()
        {
        if (ipConnectionMonitor)
            ipConnectionMonitor->Detach(this);
        
        delete ipConnectionMonitor;

        if (ipInterfaceMonitor)
            ipInterfaceMonitor->Detach(this);
        
        delete ipInterfaceMonitor;
        
        delete ipUtils;
        delete ipCommsDb;

        Close();
        
        iSocketServer.Close();
        iObservers.Close();
        }

    
    static CImp* NewL(CWLAN* apOwner)
        {
        CImp
            *pThis = NewLC(apOwner);
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }


    static CImp* NewLC(CWLAN* apOwner)
        {
        CImp
            *pThis = new (ELeave) CImp(apOwner);
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL();

        return pThis;
        }
    
    
    void ActivateL()
        {
        Close();

        User::LeaveIfError(iConnection.Open(iSocketServer));

        TInt
            rc;
        TConnectionInfo
            info;

        if (ActiveL(info))
            {
            RConnection::TConnAttachType
                type = RConnection::EAttachTypeNormal;

            rc = iConnection.Attach(TPckg<TConnectionInfo>(info), type);

            User::LeaveIfError(rc);
            }
        else
            {
            TCommDbConnPref
                prefs;
            TUint32
                bearers = 0;
            
            bearers |= ECommDbBearerWLAN;
            
            prefs.SetBearerSet(bearers);
            prefs.SetDialogPreference(ECommDbDialogPrefPrompt);
            prefs.SetDirection(ECommDbConnectionDirectionIncoming);
            
            rc = iConnection.Start(prefs);

            switch (rc)
                {
                case KErrNone:
                case KErrCancel:
                    break;

                default:
                    User::Leave(rc);
                }
            }
        
        if (rc == KErrNone)
            iActivated = true;
        }


    void Close()
        {
        iActivated = false;
        
        iConnection.Close();
        }
    
    
    TBool ActiveL() const
        {
        TConnectionInfo
            info;
        
        return ActiveL(info);
        }


    TBool ActiveL(TConnectionInfo& aInfo) const
        {
        CImp
            *pThis = const_cast<CImp*>(this);
        
        return WLANActiveL(pThis->iSocketServer, aInfo);
        }

    
    TBool ActivatedL() const
        {
        return iActivated;
        }


    void  AttachL(MObserver* apObserver)
        {
        iObservers.AppendL(apObserver);
        
        TConnectionInfo
            info;
        
        if (ipConnectionMonitor->Monitoring(info))
            {
            TNifProgress
                progress;
            
            progress.iStage = KConnectionOpen;
            
            TConnectionStatus
                status(info, progress);
            
            TRAP_IGNORE(apObserver->UpdateL(&iOwner, status));
            }
        }


    void Detach(MObserver* apObserver)
        {
        TInt
            i = iObservers.Count() - 1;
                
        while (i >= 0)
            {
            if (iObservers[i] == apObserver)
                break;
            else
                --i;
            }
        
        if (i >= 0)
            iObservers.Remove(i);
        }


    TBool Attached(MObserver* apObserver) const
        {
        TInt
            i = 0;
                
        while (i < iObservers.Count())
            {
            if (iObservers[i] == apObserver)
                return ETrue;
            else
                ++i;
            }
        
        return EFalse;
        }

    
    static void GetIpAddressesL(RSocketServ&           aServer,
                                const TConnectionInfo& aInfo,
                                RArray<TInetAddr>&     aAddresses)
        {
        RConnection
            connection;
        
        User::LeaveIfError(connection.Open(aServer));
        CleanupClosePushL(connection);
        User::LeaveIfError(connection.Attach(TPckg<TConnectionInfo>(aInfo), 
                                             RConnection::EAttachTypeNormal)); 
        
        RSocket
            socket;
        
        User::LeaveIfError(socket.Open(aServer, 
                                       KAfInet, KSockStream, KProtocolInetTcp, 
                                       connection));
        CleanupClosePushL(socket);
        
        TSoInetInterfaceInfo
            ifInfo;
        TPckg<TSoInetInterfaceInfo>
            ifInfoPckg(ifInfo);
        TSoInetIfQuery
            ifQuery;
        TPckg<TSoInetIfQuery>
            ifQueryPckg(ifQuery);
        
        User::LeaveIfError(socket.SetOpt(KSoInetEnumInterfaces, 
                                         KSolInetIfCtrl));
        
        while (socket.GetOpt(KSoInetNextInterface, KSolInetIfCtrl, 
                             ifInfoPckg) == KErrNone)
            {
            ifQuery.iName = ifInfo.iName;
            
            TInt
                rc = socket.GetOpt(KSoInetIfQueryByName, 
                                   KSolInetIfQuery, ifQueryPckg);
            
            if ((rc == KErrNone) && (ifQuery.iZone[1] == aInfo.iIapId))
                aAddresses.AppendL(ifInfo.iAddress);
            }
        
        CleanupStack::PopAndDestroy(); // socket
        CleanupStack::PopAndDestroy(); // connection
        }


    void GetIpAddressesL(const TConnectionInfo& aInfo,
                         RArray<TInetAddr>&     aAddresses)
        {
        GetIpAddressesL(iSocketServer, aInfo, aAddresses);
        }
    

    TBool GetIpAddressL(const TConnectionInfo& aInfo, TDes& aText)
        {
        RArray<TInetAddr>
            addresses;
        
        CleanupClosePushL(addresses);
        
        GetIpAddressesL(aInfo, addresses);
        
        TInetAddr
            candidate;
        
        for (TInt i = 0; i < addresses.Count(); ++i)
            {
            TInetAddr
                address = addresses[i];
            
            if (!address.IsUnspecified())
                {
                if (candidate.IsUnspecified())
                    // Any address is better than no address.
                    candidate = address;
                else if (address.IsV4Mapped() && !address.IsLinkLocal())
                    // An IPV4-mapped address is better than any other
                    // address, except if it is link local.
                    candidate = address;
                }
            }
        
        if (!candidate.IsUnspecified()) 
            candidate.Output(aText);
        
        CleanupStack::PopAndDestroy(); // addresses
    
        return !candidate.IsUnspecified();
        }
    

    void GetApNameL(const TUint32 aIapId, TDes& aName)
        {
        ipUtils->NameL(aIapId, aName);
        }

private:
    void UpdateL(CConnectionMonitor*,
                 const TNifProgress& aProgress)
        {
        TConnectionInfo
            info;
        
        ipConnectionMonitor->Monitoring(info);
        
        TConnectionStatus
            status(info, aProgress);
        
        Notify(status);
        }
    
    
    void UpdateL(CInterfaceMonitor*, 
                 const TInterfaceNotification& aNotification)
        {
        if (aNotification.iState == EInterfaceUp)
            {
            TUint32
                iapId = aNotification.iConnectionInfo.iIapId;
            TApBearerType
                type = EApBearerTypeCSD;
            
            TRAPD(rc, type = ipUtils->BearerTypeL(iapId));
            
            if ((rc == KErrNone) && (type == EApBearerTypeWLAN))
                {
                if (ipConnectionMonitor->Monitoring())
                    {
                    // How can this happen. A WLAN interface went up,
                    // with another one already up?
                    
                    ipConnectionMonitor->Stop();
                    }
                
                ipConnectionMonitor->StartL(aNotification.iConnectionInfo);
                
                TNifProgress
                    progress;
                
                progress.iStage = KConnectionOpen;
                
                Notify(TConnectionStatus(aNotification.iConnectionInfo,
                                         progress));
                }
            }
        }
        
    
    void ErrorL(CInterfaceMonitor*, TInt /*aError*/)
        {
        // Not much we can do, so let's ignore it for the time being.
        }
    
    
    void Notify(const TConnectionStatus& aData)
        {
        // Count() is called during the loop by design. This way things 
        // will not break even if observers are attached/detached during 
        // the iteration. The result will be unpredictable, but there'll 
        // be no crash.
        
        for (TInt i = 0; i < iObservers.Count(); ++i)
            {
            MObserver
                *pObserver = iObservers[i];
            
            TRAP_IGNORE(pObserver->UpdateL(&iOwner, aData));
            }
        }
    
    
private:
    CImp(CWLAN* apOwner) 
        : iOwner(*apOwner)
        {
        }
    
    void ConstructL()
        {
        User::LeaveIfError(iSocketServer.Connect());
        
        ipCommsDb           = CCommsDatabase::NewL();
        ipUtils             = CApUtils::NewLC(*ipCommsDb);
        CleanupStack::Pop(ipUtils);
        ipInterfaceMonitor  = CInterfaceMonitor::NewL(&iSocketServer, this);
        ipConnectionMonitor = CConnectionMonitor::NewL(&iSocketServer);
        
        ipInterfaceMonitor->AttachL(this);
        ipConnectionMonitor->AttachL(this);
        
        TConnectionInfo
            info;
        TBool
            active = false;
        
        TRAP_IGNORE(active = ActiveL(info));
        
        if (active)
            ipConnectionMonitor->StartL(info);
        }

private:
    CWLAN&                   iOwner;
    ::RSocketServ            iSocketServer;
    ::RConnection            iConnection;
    ::TBool                  iActivated;
    CCommsDatabase*          ipCommsDb;
    CApUtils*                ipUtils;
    CInterfaceMonitor*       ipInterfaceMonitor;
    CConnectionMonitor*      ipConnectionMonitor;
    RPointerArray<MObserver> iObservers;
    };

//
// CWLAN
//

CWLAN::CWLAN()
    {
    }



CWLAN::~CWLAN()
    {
    delete ipImp;
    }
    


void CWLAN::ConstructL()
    {
    ipImp = CImp::NewL(this);
    }


CWLAN* CWLAN::NewL()
    {
    CWLAN
        *pThis = NewLC();
    
    CleanupStack::Pop(pThis);
    
    return pThis;
    }



CWLAN* CWLAN::NewLC()
    {
    CWLAN
        *pThis = new (ELeave) CWLAN;

    CleanupStack::PushL(pThis);
    pThis->ConstructL();

    return pThis;
    }
    

void CWLAN::ActivateL()
    {
    ipImp->ActivateL();
    }


void CWLAN::Close()
    {
    ipImp->Close();
    }
    

TBool CWLAN::ActiveL() const
    {
    return ipImp->ActiveL();
    }


TBool CWLAN::ActiveL(TConnectionInfo& aInfo) const
    {
    return ipImp->ActiveL(aInfo);
    }
    

TBool CWLAN::ActivatedL() const
    {
    return ipImp->ActivatedL();
    }


void CWLAN::AttachL(MObserver* apObserver)
    {
    ipImp->AttachL(apObserver);
    }


void CWLAN::Detach(MObserver* apObserver)
    {
    ipImp->Detach(apObserver);
    }


TBool CWLAN::Attached(MObserver* apObserver) const
    {
    return ipImp->Attached(apObserver);
    }


void CWLAN::GetIpAddressesL(RSocketServ&           aServer,
                            const TConnectionInfo& aInfo,
                            RArray<TInetAddr>&     aAddresses)
    {
    CImp::GetIpAddressesL(aServer, aInfo, aAddresses);
    }


void CWLAN::GetIpAddressesL(const TConnectionInfo& aInfo,
                            RArray<TInetAddr>&     aAddresses)
    {
    ipImp->GetIpAddressesL(aInfo, aAddresses);
    }


TBool CWLAN::GetIpAddressL(const TConnectionInfo& aInfo, TDes& aText)
    {
    return ipImp->GetIpAddressL(aInfo, aText);
    }


void CWLAN::GetApNameL(const TUint32 aIapId, TDes& aName)
    {
    ipImp->GetApNameL(aIapId, aName);
    }
