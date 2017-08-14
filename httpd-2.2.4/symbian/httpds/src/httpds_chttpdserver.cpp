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

#include "httpds/chttpdserver.h"
#include "httpds/chttpd.h"
#include "httpds/csettings.h"
#include <bacline.h>
#include <es_sock.h>
#include <in_sock.h>
#include <fcntl.h> 
#include <unistd.h>
#define CORE_PRIVATE
#include <httpd.h>
#include <httpd.h>
#include <http_log.h>
extern "C"
{
#include <ap_mpm.h>
}

using namespace Mws;
using namespace Httpd;

extern "C" IMPORT_C int apache_main(int argc, const char * const argv[]);
extern "C" void exit(int ret);

extern "C" IMPORT_C void ap_symbian_shutdown(int is_graceful);
extern "C" IMPORT_C void ap_symbian_restart(int is_graceful);
extern "C" IMPORT_C int  ap_symbian_mpm_init(void (*)(void*), void* data);
extern "C" IMPORT_C void ap_symbian_mpm_finish(void);


namespace
{
    enum
        {
        KGranularity = 8
        };


    const TInt KPanicKernExec = 3;
    const TInt KPanicUser42   = 42;

    _LIT(KSettingsFileName, "httpds.dat");
}


namespace HttpdS
{
    using namespace Mws;
    

    //
    // CThreadWaiter waits for the thread in which Apache proper is running to 
    // finish. It then updates the state information of CHttpdServer.
    // 
    
    class CHttpdServer::CThreadWaiter : public ::CActive
        {
    public:
        ~CThreadWaiter()
            {
            Cancel();
            }

        static CThreadWaiter* NewL(CHttpdServer* apServer)
            {
            CThreadWaiter
                *pThis = NewLC(apServer);
            
            CleanupStack::Pop(pThis);
            
            return pThis;
            }
        
        
        static CThreadWaiter* NewLC(CHttpdServer* apServer)
            {
            CThreadWaiter
                *pThis = new (ELeave) CThreadWaiter(apServer);
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();
            
            return pThis;
            }

        TInt WaitOn(TThreadId aThreadId)
            {
            TInt
                rc = KErrInUse;
            
            if (!IsActive())
                {
                iThread.Close();
                
                rc = iThread.Open(aThreadId);
                
                if (rc == KErrNone)
                    {
                    iThread.Logon(iStatus);

                    SetActive();
                    }
                }

            return rc;
            }

        TThreadId WaitingOn() const
            {
            return iThread.Id();
            }
        
    protected:
        void DoCancel()
            {
            iThread.LogonCancel(iStatus);
            iThread.Close();
            }

        
        void RunL()
            {
            iServer.ThreadDeath(iThread.Id(), 
                                iThread.ExitType(),
                                iThread.ExitReason());
            
            iThread.Close();
            }
        
    private:
        CThreadWaiter(CHttpdServer* apServer) : 
            CActive(EPriorityStandard),
            iServer(*apServer)
            {
            }

        void ConstructL()
            {
            CActiveScheduler::Add(this);
            }

    private:
        CHttpdServer& iServer;
        RThread       iThread;
        };


    //
    // CPoller polls httpd until the state has changed to
    // the expected one.
    // 

    class CHttpdServer::CPoller : public ::CTimer
        {
    public:
        ~CPoller()
            {
            Cancel();
            }

        static CPoller* NewL(CHttpdServer* apServer)
            {
            CPoller
                *pThis = NewLC(apServer);
            
            CleanupStack::Pop(pThis);
            
            return pThis;
            }
        
        
        static CPoller* NewLC(CHttpdServer* apServer)
            {
            CPoller
                *pThis = new (ELeave) CPoller(apServer);
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();
            
            return pThis;
            }

        
        TInt Poll()
            {
            TInt
                rc = KErrNone;
            
            // Poll only if we are not already doing it.
            
            if (!IsActive())
                RunL();
            
            return rc;
            }
        
    protected:
        void RunL()
            {
            int
                result;

            (void) ap_mpm_query(AP_MPMQ_MPM_STATE, &result);

            if (result != AP_MPMQ_RUNNING)
                // Poll once a second.
                After(TTimeIntervalMicroSeconds32(1000000));
            else
                iServer.UpdateHttpdState(EHttpdRunning);
            }
        
    private:
        CPoller(CHttpdServer* apServer) : 
            CTimer(EPriorityHigh),
            iServer(*apServer)
            {
            }

        void ConstructL()
            {
            CTimer::ConstructL();
            
            CActiveScheduler::Add(this);
            }

    private:
        CHttpdServer& iServer;
        };

    
    class CHttpdServer::CWorkerOverseer : public ::CActive
        {
    public:
        ~CWorkerOverseer()
            {
            Cancel();
            }

        static CWorkerOverseer* NewL(CHttpdServer* apServer,
                                     RSemaphore*   apSemaphore)
            {
            CWorkerOverseer
                *pThis = NewLC(apServer, apSemaphore);
            
            CleanupStack::Pop(pThis);
            
            return pThis;
            }
        
        
        static CWorkerOverseer* NewLC(CHttpdServer* apServer,
                                      RSemaphore*   apSemaphore)
            {
            CWorkerOverseer
                *pThis = new (ELeave) CWorkerOverseer(apServer, apSemaphore);
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();
            
            return pThis;
            }


        void Activate()
            {
            iStatus = KRequestPending;
            SetActive();
            
            iSemaphore.Signal();
            }


        
        /* This MUST only be called from another thread but the main thread.
         */
        static void WorkerCreated(void* pThis)
            {
            static_cast<CWorkerOverseer*>(pThis)->WorkerCreated();
            }

    protected:
        void DoCancel()
            {
            TRequestStatus
                *pStatus = &iStatus;
            
            /* RunL() will not be called because of this.
             */
            RThread().RequestComplete(pStatus, 0);
            }

        
        void RunL()
            {
            iServer.WorkerCreated(iWorker);
            
            Activate();
            }
        
    private:
        CWorkerOverseer(CHttpdServer* apServer,
                        RSemaphore*   apSemaphore) : 
            CActive(EPriorityStandard),
            iServer(*apServer),
            iSemaphore(*apSemaphore)
            {
            }

        void ConstructL()
            {
            CActiveScheduler::Add(this);
            
            iThreadId = RThread().Id();
            }


        void WorkerCreated()
            {
            iSemaphore.Wait();
            
            iWorker = RThread().Id();
            
            RThread
                thread;
            
            thread.Open(iThreadId);

            TRequestStatus
                *pStatus = &iStatus;
            
            /* This causes RunL() above to be called.
             */
            thread.RequestComplete(pStatus, 0);

            thread.Close();
            }

    private:
        TThreadId     iThreadId;
        CHttpdServer& iServer;
        RSemaphore&   iSemaphore;
        TThreadId     iWorker;
        };
}


namespace
{

    void BuildCommandLineL(char*         pCommandLine,
                           const char*   argv0,
                           const char**& argv,
                           int&          argc)
        {
        char
            *end   = pCommandLine + strlen(pCommandLine),
            *i     = pCommandLine;

        argc = 1;
        
        bool quoting = false;
        bool spacing = true;
        
        while (i != end)
            {
            switch (*i)
                {
                case ' ':
                case '\t':
                    if (!quoting)
                        {
                        *i = 0;
                        spacing = true;
                        }
                    break;
                    
                case '"':
                    *i = 0;
                    if (quoting)
                        quoting = false;
                    else
                        {
                        ++argc;
                        quoting = true;
                        }
                    break;
                    
                default:
                    if (!quoting && spacing)
                        {
                        ++argc;
                        spacing = false;
                        }
                }
            
            ++i;
            }

        TInt
            size = sizeof(const char*) * (argc + 1); // One for NULL

        argv = static_cast<const char**>(User::Alloc(size));

        int
            arg = 0;
        
        argv[arg++] = argv0;

        i = pCommandLine;

        while (arg != argc)
            {
            while ((*i == 0) && (i != end))
                ++i;

            argv[arg++] = i;

            while ((*i != 0) && (i != end))
                ++i;
            }

        argv[argc] = 0;
        }


    void GetHttpdRoot(const char** argv, int argc, const char*& httpdRoot)
        {
        int
            i = 1;
        
        while (i < argc)
            {
            if (strcmp(argv[i], "-d") == 0)
                break;

            ++i;
            }

        if (i == argc)
            // No root specified, so use the default one.
            httpdRoot = HTTPD_ROOT;
        else if (i < argc - 1)
            // Use specified root.
            httpdRoot = argv[i + 1];
        else
            // No argument provided for "-d".
            httpdRoot = "C:";
        }


    void RunL(char* pCommandLine)
        {
        TFileName
            fileName = RProcess().FileName();
        HBufC8
            *pFileName = HBufC8::NewLC(fileName.Length() + 1);

        pFileName->Des().Copy(fileName);

        const char
            *argv0 = reinterpret_cast<const char*>(pFileName->Des().PtrZ());

        int
            argc;
        const char
            **argv;

        BuildCommandLineL(pCommandLine, argv0, argv, argc);

        CleanupStack::PushL(argv);

        const char
            *httpdRoot;
        
        GetHttpdRoot(argv, argc, httpdRoot);

        // Redirect stderr to a file.
        
        TInt
            size = strlen(httpdRoot) + sizeof("/logs/stderr.log");
        char
            *path = static_cast<char*>(User::Alloc(size));

        if (!path)
            User::Leave(KErrNoMemory);

        CleanupStack::PushL(path);

        strcpy(path, httpdRoot);
        strcat(path, "/logs/stderr.log");

        int
            flags = O_CREAT | O_APPEND | O_RDWR;
        int
            fd = open(path, flags);

        if (fd == -1)
            {
            HttpdS::WriteLog(_L("Failed to open logs/stderr.log."));
            User::Leave(KErrGeneral);
            }

        int
            tmp = dup(STDERR_FILENO); // Save

        // According to OpenC's documentation dup2 may succeed without
        // the returned descriptor being the same as the one passed as 
        // second argument. There's nothing we can do in that case, so 
        // we just ignore the possibility.

        dup2(fd, STDERR_FILENO); // Redirect
        close(fd);

        apache_main(argc, argv);

        dup2(tmp, STDERR_FILENO); // Restore
        close(tmp);

        CleanupStack::PopAndDestroy(path);
        CleanupStack::PopAndDestroy(argv);        
        CleanupStack::PopAndDestroy(pFileName);
        }

    
    void Run(char* pCommandLine)
        {
        TRAP_IGNORE(RunL(pCommandLine));
        }


    void SetupAndRunL(TAny* apAny)
        {
        char
            *pCommandLine = static_cast<char*>(apAny);
        
        CleanupStack::PushL(pCommandLine);

        CActiveScheduler
            *pCurrent   = CActiveScheduler::Current(), // Should always be 0.
            *pScheduler = 0;
        
        if (!pCurrent)
            {
            pScheduler = new (ELeave) CActiveScheduler;
            
            CActiveScheduler::Install(pScheduler);
            }
        
        Run(pCommandLine);
        
        if (pScheduler)
            {
            CActiveScheduler::Install(0);

            TVersion
                version = User::Version();
            TInt
                major = version.iMajor,
                minor = version.iMinor,
                build = version.iBuild;

            // For these magic numbers please consult e32ver.h. All Symbian 
            // 9.X releases seem to have 2 as major number and 0 as minor. 
            // From the build number the Symbian release minor version can 
            // be deduced:
            //
            //   9.1 <=> 1045
            //   9.2 <=> 1150
            //   9.3 <=> 1540
            // 
            // Here we assume that a build number >= 1540 means 9.3

            if ((major == 2) && (minor == 0) && (build < 1540))
                {
                // On 9.3, for an unknown reason, the deletion of the scheduler
                // never returns. My unsubstantiated guess is that there is an
                // active object that has not been removed and the deletion
                // expects it to be removed before completing. Thus, so as to
                // prevent hanging on 9.3, only on 9.1 and 9.2 we actually 
                // delete the instance.
                
                delete pScheduler;
                }
            }

        CleanupStack::PopAndDestroy(pCommandLine);
        }


    TInt HttpdMain(TAny* apAny)
        {
        TInt
            rc = KErrNone;
        
        CTrapCleanup
            *pCleanupStack = CTrapCleanup::New();
        
        if (pCleanupStack == 0)
            HttpdS::CHttpdServer::PanicServer(EHttpdPanicServerCreate);

        TRAP(rc, SetupAndRunL(apAny));
        
        delete pCleanupStack;

        return rc;
        }


    namespace
        {
            void ReportL(const Mws::THttpdSettings& aSettings)
                {
                _LIT(KFormat, 
                     "Starting httpd with "
                     L"stacksize = 0x%x, minheapsize = 0x%x, and "
                     L"maxheapsize = 0x%x.");

                HBufC
                    *pBuffer = HBufC::NewLC(KFormat.iTypeLength + 3 * 10);
                
                pBuffer->Des().Format(KFormat,
                                      aSettings.StackSize(), 
                                      aSettings.MinHeapSize(), 
                                      aSettings.MaxHeapSize());
                
                HttpdS::WriteLog(*pBuffer);
                
                CleanupStack::PopAndDestroy(pBuffer);
                }
        }


    void DoStartHttpdL(RThread*                             apThread,
                       const TDesC&                         aCommandLine,
                       const Mws::THttpdSettings&           aSettings,
                       HttpdS::CHttpdServer::CThreadWaiter& aThreadWaiter)
        
        {
        ReportL(aSettings);

        HBufC8
            *pCommandLine8 = HBufC8::NewLC(aCommandLine.Length() * 2 + 1);

        pCommandLine8->Des().Copy(aCommandLine);
        
        RHeap
            *pHeap = UserHeap::ChunkHeap(0,
                                         aSettings.MinHeapSize(),
                                         aSettings.MaxHeapSize());
        
        if (!pHeap)
            User::Leave(KErrNoMemory);
        
        TInt
            size = pCommandLine8->Length() + 1; // One for the NULL
        char
            *pCommandLine = static_cast<char*>(pHeap->Alloc(size));
        
        if (!pCommandLine)
            {
            pHeap->Close();
            
            User::Leave(KErrNoMemory);
            }

        strcpy(pCommandLine, 
               reinterpret_cast<const char*>(pCommandLine8->Des().PtrZ()));
        
        CleanupStack::PopAndDestroy(pCommandLine8);
        
        TInt
            rc = apThread->Create(_L("HttpdMain"),
                                  HttpdMain,
                                  aSettings.StackSize(),
                                  pHeap,
                                  pCommandLine);

        if (rc != KErrNone)
            {
            pHeap->Free(pCommandLine);
        
            pHeap->Close();
        
            User::Leave(rc);
            }

        // Failures are ignored. We simply won't notice when it exits.
        aThreadWaiter.WaitOn(apThread->Id());
        
        apThread->Resume();
        }
}


namespace
{
    using namespace HttpdS;
    using namespace Mws;
    
    
    void CreatePrivateDirectoryL(TDes& aPrivatePath)
        {
        RFs
            hFs;
        
        User::LeaveIfError(hFs.Connect());
        CleanupClosePushL(hFs);

        TChar
            drive = RProcess().FileName()[0];
        TInt
            driveNumber;
        
        RFs::CharToDrive(drive, driveNumber);

        TDriveInfo
            info;

        User::LeaveIfError(hFs.Drive(info, driveNumber));

        if (info.iMediaAtt & KMediaAttWriteProtected)
            {
            // If the drive is write protected - the software has been
            // installed in ROM - we create the private path on C:
            driveNumber = EDriveC;

            User::LeaveIfError(hFs.DriveToChar(driveNumber, drive));
            }
        
        User::LeaveIfError(hFs.CreatePrivatePath(driveNumber));

        TFileName
            path;
        
        User::LeaveIfError(hFs.PrivatePath(path));

        aPrivatePath.Zero();

        aPrivatePath.Append(drive);
        aPrivatePath.Append(_L(":"));
        aPrivatePath.Append(path);
        
        CleanupStack::PopAndDestroy(); // hFs
        }


    THttpdSettings GetSettings(const THttpdSettings& aSettings,
                               const CSettings&      aXmlSettings)
        {
        TInt
            stackSize, minHeapSize, maxHeapSize;

        if (aXmlSettings.Override())
            {
            stackSize   = aXmlSettings.StackSize();
            minHeapSize = aXmlSettings.MinHeapSize();
            maxHeapSize = aXmlSettings.MaxHeapSize();
            }
        else
            {
            stackSize   = aSettings.StackSize();
            minHeapSize = aSettings.MinHeapSize();
            maxHeapSize = aSettings.MaxHeapSize();
            
            if (stackSize == 0)
                stackSize = HttpdS::KDefaultStackSize;
            
            if (minHeapSize == 0)
                minHeapSize = HttpdS::KDefaultMinHeapSize;
            
            if (maxHeapSize == 0)
                maxHeapSize = HttpdS::KDefaultMaxHeapSize;
            
            // Unless overriding, the values from the xml-file can only
            // increase the amount.
            
            if (aXmlSettings.StackSize() > stackSize)
                stackSize = aXmlSettings.StackSize();
            
            if (aXmlSettings.MinHeapSize() > minHeapSize)
                minHeapSize = aXmlSettings.MinHeapSize();
            
            if (aXmlSettings.MaxHeapSize() > maxHeapSize)
                maxHeapSize = aXmlSettings.MaxHeapSize();
            }


        if (minHeapSize > maxHeapSize)
            minHeapSize = maxHeapSize;
        
        return THttpdSettings(minHeapSize, maxHeapSize, stackSize);
        }


    THttpdSettings GetSettings(const THttpdSettings& aUserSettings,
                               const THttpdSettings& aStoredSettings,
                               const CSettings&      aXmlSettings)
        {
        THttpdSettings
            settings;
        
        if (aUserSettings.MaxHeapSize() != 0)
            settings.SetMaxHeapSize(aUserSettings.MaxHeapSize());
        else
            settings.SetMaxHeapSize(aStoredSettings.MaxHeapSize());

        if (aUserSettings.MinHeapSize() != 0)
            settings.SetMinHeapSize(aUserSettings.MinHeapSize());
        else
            settings.SetMinHeapSize(aStoredSettings.MinHeapSize());

        if (aUserSettings.StackSize() != 0)
            settings.SetStackSize(aUserSettings.StackSize());
        else
            settings.SetStackSize(aStoredSettings.StackSize());

        return GetSettings(settings, aXmlSettings);
        }


    THttpdSettings GetSettingsL(const THttpdSettings& aUserSettings,
                                const THttpdSettings& aStoredSettings)
        {
        HttpdS::CSettings
            *pSettings = HttpdS::CSettings::NewLC();
        TInt
            rc = pSettings->ReadL(HTTPD_ROOT "/httpds.xml");
        
        if ((rc != KErrNone) && (rc != KErrNotFound))
            HttpdS::WriteLog(_L("Reading settings from httpds.xml failed. "
                                L"Any read values will be ignored."));

        THttpdSettings
            settings = GetSettings(aUserSettings, 
                                   aStoredSettings,
                                   *pSettings);
        
        CleanupStack::PopAndDestroy(pSettings);

        return settings;
        }

}


namespace HttpdS
{
    CHttpdServer::CHttpdServer(TInt aPriority) 
        : ::CServer2(aPriority),
          iState(EHttpdNotStarted)
        {
        }
    
    
    CHttpdServer::~CHttpdServer()
        {
        delete ipPoller;
        ipPoller = 0;

        delete ipThreadWaiter;
        ipThreadWaiter = 0;

        delete ipSessions;
        ipSessions = 0;

        delete ipWaiters;
        ipWaiters = 0;
        
        ap_symbian_mpm_finish();

        delete ipWorkerOverseer;
        ipWorkerOverseer = 0;
        
        iSemaphore.Close();
        }
    
    
    CHttpdServer* CHttpdServer::NewL(TInt aPriority)
        {
        CHttpdServer
            *pThis = NewLC(aPriority);
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }
    

    CHttpdServer* CHttpdServer::NewLC(TInt aPriority)
        {
        CHttpdServer
            *pThis = new (ELeave) CHttpdServer(aPriority);
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL();
        return pThis;
        }

    
    void CHttpdServer::ConstructL()
        {
        CreatePrivateDirectoryL(iPrivatePath);

        ReadSettingsL();

        User::LeaveIfError(iSemaphore.CreateLocal(0));

        ipWorkerOverseer = CWorkerOverseer::NewL(this, &iSemaphore);
        
        if (ap_symbian_mpm_init(&CWorkerOverseer::WorkerCreated, 
                                ipWorkerOverseer) != 0)
            User::Leave(KErrGeneral);

        ipSessions = new (ELeave) CArrayPtrSeg<CHttpd>(KGranularity);
        ipWaiters = new (ELeave) CArrayPtrFlat<CThreadWaiter>(KGranularity);

        ipThreadWaiter = CThreadWaiter::NewL(this);
        ipPoller = CPoller::NewL(this);
        
        StartL(KServerName);
        }
    

    void CHttpdServer::ReadSettingsL()
        {
        TFileName
            path;
        
        path.Append(iPrivatePath);
        path.Append(KSettingsFileName);

        TInt
            rc;
        RFs
            hFs;
        
        rc = hFs.Connect();
        
        User::LeaveIfError(rc);
        CleanupClosePushL(hFs);
        
        RFile
            hFile;

        rc = hFile.Open(hFs, path, EFileRead);
        
        switch (rc)
            {
            case KErrNone:
            case KErrNotFound:
                break;

            default:
                User::Leave(rc);
            }
            
        if (rc == KErrNone)
            {
            CleanupClosePushL(hFile);
                
            RFileReadStream
                hStream;
            
            hStream.Attach(hFile);
            hStream.PushL();
            
            iSettings.InternalizeL(hStream);
            
            CleanupStack::PopAndDestroy(2); // hStream, hFile
            }

        CleanupStack::PopAndDestroy(); // hFs
        }


    void CHttpdServer::PanicServer(THttpdPanic aCode)
        {
        User::Panic(KServerFileName, aCode);
        }


    namespace
    {
        TInt FindSession(const CArrayPtr<CHttpd>& aSessions,
                         CHttpd*                  apSession)
            {
            TInt
                i = -1;
            
            for (TInt j = 0; j < aSessions.Count(); j++)
                {
                if (aSessions.At(j) == apSession)
                    {
                    i = j;
                    
                    break;
                    }
                }
            
            return i;
            }
    }


    void CHttpdServer::AttachL(CHttpd* apSession)
        {
        ASSERT(FindSession(*ipSessions, apSession) == -1);
        
        ipSessions->AppendL(apSession);
        }
    
    
    void CHttpdServer::Detach(CHttpd* apSession)
        {
        /* If ipSessions is NULL, then this is called when the server
         * object itself is being deleted and the session objects are
         * being killed by the destructor of CServer2.
         */
        
        if (ipSessions)
            {
            TInt
                i = FindSession(*ipSessions, apSession);
            
            ASSERT(i != -1);
            
            ipSessions->Delete(i);
            
            if (ipSessions->Count() == 0)
                {
                if (iShouldExit)
                    iState = EHttpdStopping;
                
                switch (iState)
                    {
                    case EHttpdNotStarted:
                    case EHttpdStopping:
                    case EHttpdStopped:
                    case EHttpdTerminated:
                        CActiveScheduler::Stop();
                        break;
                        
                    default:
                        ;
                    }
                }
            }
        }


    TInt CHttpdServer::StartHttpdL(const TDesC&     aCommandLine,
                                   const TSettings& aSettings)
        {
        TInt
            rc = KErrNone;

        // Only a non-started HTTPD can be started. In order to be
        // able to start a stopped HTTPD, all static data of HTTPD
        // would have to be cleaned and that simply is not worth
        // the trouble.

        if (iState == EHttpdNotStarted)
            {
            ipWaiters->SetReserveL(ipWaiters->Count() + 1);
                
            CThreadWaiter
                *pWaiter = CThreadWaiter::NewLC(this);

            UpdateHttpdState(EHttpdStarting);

            iCurrentSettings = GetSettingsL(aSettings, iSettings);

            if (!ipWorkerOverseer->IsActive())
                ipWorkerOverseer->Activate();
            
            TRAP(rc, DoStartHttpdL(&iThread,
                                   aCommandLine, 
                                   iCurrentSettings, 
                                   *pWaiter));
            
            if (rc == KErrNone) 
                {
                ipWaiters->AppendL(pWaiter); // Will not Leave.
                CleanupStack::Pop(pWaiter);
            
                ipPoller->Poll();
                }
            else
                {
                CleanupStack::PopAndDestroy(pWaiter);
                
                UpdateHttpdState(EHttpdNotStarted);
                }
            }
        else
            rc = KErrInUse;
        
        return rc;
        }
    
    
    TInt CHttpdServer::StopHttpd(TBool aGraceful)
        {
        if (iState == EHttpdRunning)
            {
            iThread.Id();
            
            ap_symbian_shutdown(aGraceful);

            UpdateHttpdState(EHttpdStopping);
            }

        iShouldExit = true;
            
        return KErrNone;
        }


    TInt CHttpdServer::RestartHttpd(TBool aGraceful)
        {
        if (iState == EHttpdRunning)
            {
            ap_symbian_restart(aGraceful);
            
            UpdateHttpdState(EHttpdRestarting);
            
            ipPoller->Poll();
            }
        
        return KErrNone;
        }


    THttpdState CHttpdServer::State() const
        {
        return iState;
        }


    TInt CHttpdServer::SetSettingsL(const THttpdSettings& aSettings)
        {
        TFileName
            path;
        
        path.Append(iPrivatePath);
        path.Append(KSettingsFileName);

        TInt
            rc;
        RFs
            hFs;
        
        rc = hFs.Connect();
        
        User::LeaveIfError(rc);
        CleanupClosePushL(hFs);
        
        RFile
            hFile;

        rc = hFile.Replace(hFs, path, EFileWrite);
        
        User::LeaveIfError(rc);
        
        CleanupClosePushL(hFile);
        
        RFileWriteStream
            hStream;
        
        hStream.Attach(hFile);
        hStream.PushL();
        
        aSettings.ExternalizeL(hStream);
        
        hStream.CommitL();
        
        CleanupStack::PopAndDestroy(3); // hStream, hFile, hFs

        iSettings = aSettings;
        
        return KErrNone;
        }


    THttpdSettings CHttpdServer::Settings() const
        {
        return iSettings;
        }


    THttpdSettings CHttpdServer::CurrentSettings() const
        {
        TBool
            running = (iState == EHttpdStarting || iState == EHttpdRunning);
        
        return running ? iCurrentSettings : iSettings;
        }


    TInt CHttpdServer::RunError(TInt aError)
        {
        // A bad descriptor error implies a badly programmed client, 
        // so panic it; otherwise report the error to the client
        
        if (aError == KErrBadDescriptor)
            {
            PanicClient(Message(), Mws::EHttpdPanicBadDescriptor);
            }
        else
            {
            Message().Complete(aError);
            }
        
        // The leave will result in an early return from CHttpdServer::RunL(), 
        // skipping the call to request another message. So do that now in 
        // order to keep the server running.
        ReStart();
        
        return KErrNone;
        }


    ::CSession2* CHttpdServer::NewSessionL(const TVersion&  aVersion,
                                           const RMessage2& /*aMessage*/) const
        {
        if (!User::QueryVersionSupported(TVersion(Httpd::KProtocolVersionMajor,
                                                  Httpd::KProtocolVersionMinor,
                                                  0),
                                         aVersion))
            {
            User::Leave(KErrNotSupported);
            }
        
        return new (ELeave) CHttpd();
        }


    void CHttpdServer::UpdateHttpdState(THttpdState aState)
        {
        iState = aState;

        switch (iState)
            {
            case EHttpdStopping:
                /* A state change to EHttpdStopping is not delivered to 
                 * a client, because that may cause the client to miss
                 * the final state change to EHttpdStopped, in which case
                 * the client will be led to believe that the server was
                 * terminated.
                 */
                break;
                
            case EHttpdStopped:
            case EHttpdTerminated:
            default:
                {
                for (TInt i = 0; i < ipSessions->Count(); ++i)
                    {
                    CHttpd
                        *pSession = ipSessions->At(i);
                    
                    TRAP_IGNORE(pSession->UpdateStateL(iState));
                    }
                
                if ((iState == EHttpdStopped) || (iState == EHttpdTerminated))
                    {
                    iThread.Close();
                
                    CActiveScheduler::Stop();
                    }
                }
            }
        }


    void CHttpdServer::WorkerCreated(TThreadId aThreadId)
        {
        TInt
            rc;

        TRAP(rc, ipWaiters->SetReserveL(ipWaiters->Count() + 1));
        
        if (rc != KErrNone)
            HttpdS::CHttpdServer::PanicServer(EHttpdPanicOutOfResources);
        
        CThreadWaiter
            *pWaiter = 0;
        
        TRAP(rc, pWaiter = CThreadWaiter::NewL(this));

        if (rc != KErrNone)
            HttpdS::CHttpdServer::PanicServer(EHttpdPanicOutOfResources);

        rc = pWaiter->WaitOn(aThreadId);

        if (rc == KErrNone)
            ipWaiters->AppendL(pWaiter); // Will not Leave.
        else
            {
            delete pWaiter;

            HttpdS::CHttpdServer::PanicServer(EHttpdPanicOutOfResources);
            }
        }


    void CHttpdServer::ThreadDeath(TThreadId aThreadId,
                                   TExitType aExitType,
                                   TInt      aExitReason)
        {
        if (aThreadId == iThread.Id())
            {
            // The main Apache thread.
            
            switch (aExitType)
                {
                case EExitKill:
                    UpdateHttpdState(EHttpdStopped);
                    break;

                case EExitTerminate:
                    ap_log_error(APLOG_MARK, APLOG_EMERG, 0, 0,
                                 "Main Apache thread terminated (%d). "
                                 "Shutting down.", aExitReason);
                    UpdateHttpdState(EHttpdTerminated);
                    break;
                    
                case EExitPanic:
                    ap_log_error(APLOG_MARK, APLOG_EMERG, 0, 0,
                                 "Main Apache thread paniced (%d). "
                                 "Shutting down.", aExitReason);
                    UpdateHttpdState(EHttpdTerminated);
                    break;
                    
                default:
                    __ASSERT_ALWAYS(false, User::Invariant());
                }
            }
        else
            {
            // Some worker thread.
            
            switch (aExitType)
                {
                case EExitKill:
                    {
                    CThreadWaiter
                        *pWaiter = 0;
                    TInt
                        i;
                    
                    for (i = 0; i < ipWaiters->Count(); ++i) 
                        {
                        pWaiter = (*ipWaiters)[i];
                        
                        if (pWaiter->WaitingOn() == aThreadId)
                            break;
                        }

                    __ASSERT_ALWAYS(i < ipWaiters->Count(),
                                    User::Invariant());

                    delete pWaiter;
                    ipWaiters->Delete(i);
                    }
                    break;
                    
                case EExitTerminate:
                    ap_log_error(APLOG_MARK, APLOG_EMERG, 0, 0, 
                                 "Worker thread terminated (%d). "
                                 "Shutting down.", aExitReason);
                    UpdateHttpdState(EHttpdTerminated);
                    break;
                    
                    
                case EExitPanic:
                    ap_log_error(APLOG_MARK, APLOG_EMERG, 0, 0,
                                 "Worker thread paniced (%d). "
                                 "Shutting down.", aExitReason);
                    UpdateHttpdState(EHttpdTerminated);
                    break;
                    
                default:
                    __ASSERT_ALWAYS(false, User::Invariant());
                }
            }
        }
}
