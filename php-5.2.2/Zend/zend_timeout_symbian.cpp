/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2007 Nokia Corporation                              	  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Mikko Heikelä <mikko.heikela@futurice.com>                  |
   +----------------------------------------------------------------------+
 */

/*
  Zend timeout implementation for Symbian OS.
*/

/*
The current timeout code is written with only one PHP thread in mind.
For the same reason, also the timeout thread creation is not protected
against multiple threads trying to create the timeout thread simulataneously.
*/

extern "C" {
#include "zend_timeout_symbian.h"
#include "zend_execute.h"

#include "php.h"
}

#include <e32def.h>
#include <e32std.h>
#include <e32err.h>
#include <e32base.h>


#ifdef ZEND_DEBUG_TIMEOUT

extern "C" {
#include <stdarg.h>
#include <stdio.h>

static void php_s60_log(const char* format, ...)
{
	FILE* logfile = fopen("C:\\Data\\Php\\php_s60_log.txt", "a");
	va_list args;
	va_start(args, format);
	vfprintf(logfile, format, args);
	va_end(args);
	fclose(logfile);
}

}

#else

static void php_s60_log(const char* format, ...)
{
}

#endif // ZEND_DEBUG_TIMEOUT

/** 
 * Values used to signal the TRequestStatus variable of the CTimeoutMessageHandler
 * active object.
 */
static const TInt KTimeoutSignalSetTimeout = 1;
static const TInt KTimeoutSignalCancelTimeout = 2;
static const TInt KTimeoutSignalKillTimeoutThread = 3;

/** Has the timeout thread already been created.  Not ready for multithreaded PHP */
static TBool timeoutThreadActive = EFalse;

/**
 * Timeout controller for the inter thread communication.
 */
class CZendTimeoutSymbianController;
static CZendTimeoutSymbianController* timeoutController = NULL;

/**
 * Handle to the timeout thread.  Should maybe be encapsulated in the above controller object.
 */
static RThread timeout_thread;

/**
 * Class for inter-thread communication between timeout thread and main PHP thread.
 */
class CZendTimeoutSymbianController : public CBase {
public:
	static CZendTimeoutSymbianController* NewL();
	static CZendTimeoutSymbianController* NewLC();
	~CZendTimeoutSymbianController();

	void SetTimeout(TInt aSeconds);
	void CancelTimeout();
	void KillTimeoutThread();
	
	/** Return the message parameter */
	TInt MessageParameter();
	/** Up the semaphore once. */
	void SignalSemaphore();
	/** Set the iTimeoutMessageHandlerStatus request status variable */
	void RegisterMessageHandler(TRequestStatus* aTimeoutMessageHandlerStatus);

private:
	// Private Symbian second phase constructor
	void ConstructL();
	// Private C++ constructor
	CZendTimeoutSymbianController();
	// Copy constructor and assignment operator not implemented
	CZendTimeoutSymbianController(const CZendTimeoutSymbianController& other);
	CZendTimeoutSymbianController& operator=(const CZendTimeoutSymbianController& other);

	/**
	 * A semaphore to synchronize the threads (and to protect the data).
	 */
	RSemaphore iReadyForMessageSemaphore;
	/**
	 * Pointer to the request status variable of the CTimeoutMessageHandler active
	 * object in the timeout thread that receives control messages from the main PHP thread.
	 */
	TRequestStatus* iTimeoutMessageHandlerStatus;
	/**
	 * Parameters for any control messages.  Currently only used for requested timeout length.
	 * when setting timeout.
	 */
	TInt iMessageParameter;
};

CZendTimeoutSymbianController* CZendTimeoutSymbianController::NewL() {
	CZendTimeoutSymbianController* result = CZendTimeoutSymbianController::NewLC();
	CleanupStack::Pop(result);
	return result;
}

CZendTimeoutSymbianController* CZendTimeoutSymbianController::NewLC() {
	CZendTimeoutSymbianController* result = new (ELeave) CZendTimeoutSymbianController();
	CleanupStack::PushL(result);
	result->ConstructL();
	return result;
}

CZendTimeoutSymbianController::~CZendTimeoutSymbianController() {
	iReadyForMessageSemaphore.Close();
}

void CZendTimeoutSymbianController::SetTimeout(TInt aSeconds) {
	php_s60_log("entering CZendTimeoutSymbianController::SetTimeout(%d)\n", aSeconds);
	php_s60_log("about to wait for semaphore in CZendTimeoutSymbianController::SetTimeout()\n");
	iReadyForMessageSemaphore.Wait();
	php_s60_log("passed semaphore in CZendTimeoutSymbianController::SetTimeout()\n");
	iMessageParameter = aSeconds;
	timeout_thread.RequestComplete(iTimeoutMessageHandlerStatus,
	                               KTimeoutSignalSetTimeout);
}

void CZendTimeoutSymbianController::CancelTimeout() {
	php_s60_log("entering CZendTimeoutSymbianController::CancelTimeout()\n");
	php_s60_log("about to wait for semaphore in CZendTimeoutSymbianController::CancelTimeout()\n");
	iReadyForMessageSemaphore.Wait();
	php_s60_log("passed semaphore in CZendTimeoutSymbianController::CancelTimeout()\n");
	timeout_thread.RequestComplete(iTimeoutMessageHandlerStatus,
	                                KTimeoutSignalCancelTimeout);
}

// Not actually called anywhere as of now?
void CZendTimeoutSymbianController::KillTimeoutThread() {
	php_s60_log("entering CZendTimeoutSymbianController::KillTimeoutThread()\n");
	php_s60_log("about to wait for semaphore in CZendTimeoutSymbianController::KillTimeoutThread()\n");
	iReadyForMessageSemaphore.Wait();
	php_s60_log("passed semaphore in CZendTimeoutSymbianController::KillTimeoutThread()\n");
	timeout_thread.RequestComplete(iTimeoutMessageHandlerStatus,
	                               KTimeoutSignalKillTimeoutThread);
	// Possibly mark that the timeout thread is not active?
}

TInt CZendTimeoutSymbianController::MessageParameter() {
	return iMessageParameter;
}

void CZendTimeoutSymbianController::SignalSemaphore()
{
	php_s60_log("entering CZendTimeoutSymbianController::SignalSemaphore()\n");
	iReadyForMessageSemaphore.Signal();
}

void CZendTimeoutSymbianController::RegisterMessageHandler(TRequestStatus* aTimeoutMessageHandlerStatus) {
	iTimeoutMessageHandlerStatus = aTimeoutMessageHandlerStatus;
}

void CZendTimeoutSymbianController::ConstructL() {
	User::LeaveIfError(iReadyForMessageSemaphore.CreateLocal(0));
}

CZendTimeoutSymbianController::CZendTimeoutSymbianController()
	: iMessageParameter(0)
{
}

/**
 * An active object class that creates a timer and waits for it
 * to reach the requested timeout.  If this happens, sets the
 * php side timeout variable.
 */
class CTimeoutTimer : public CActive {
public:
	static CTimeoutTimer* NewL();
	static CTimeoutTimer* NewLC();
	virtual ~CTimeoutTimer();

	/** 
	 * Requests a timeout.  Any previous timeout is cancelled.
	 */
	void RequestTimeout(TInt aSeconds);

protected:
	void RunL();
	void DoCancel();

private:
	// Private Symbian second phase constructor
	void ConstructL();
	// Private C++ constructor
	CTimeoutTimer();
	// Copy constructor and assignment operator not implemented
	CTimeoutTimer(const CTimeoutTimer& other);
	CTimeoutTimer& operator=(const CTimeoutTimer& other);

	RTimer iTimer;
};

CTimeoutTimer* CTimeoutTimer::NewL() {
	CTimeoutTimer* result = CTimeoutTimer::NewLC();
	CleanupStack::Pop(result);
	return result;
}

CTimeoutTimer* CTimeoutTimer::NewLC() {
	CTimeoutTimer* result = new (ELeave) CTimeoutTimer();
	CleanupStack::PushL(result);
	result->ConstructL();
	return result;
}

void CTimeoutTimer::RequestTimeout(TInt aSeconds) {
	php_s60_log("entering CTimeoutTimer::RequestTimeout(%d)\n", aSeconds);
	if( IsActive() ) {
		Cancel();
	}
	const TInt secondsToMilliseconds = 1000 * 1000;
	php_s60_log("about to call iTimer.After() in CTimeoutTimer::RequestTimeout(%d)\n", aSeconds);
	iTimer.After(iStatus, aSeconds * secondsToMilliseconds);
	SetActive();
	php_s60_log("exiting CTimeoutTimer::RequestTimeout(%d)\n", aSeconds);
}

CTimeoutTimer::CTimeoutTimer()
	: CActive(EPriorityStandard)
{
	CActiveScheduler::Add(this);
}

void CTimeoutTimer::ConstructL() {
	User::LeaveIfError(iTimer.CreateLocal());
}

CTimeoutTimer::~CTimeoutTimer() {
	Cancel();
	iTimer.Close();
}

void CTimeoutTimer::RunL() {
	php_s60_log("entering CTimeoutTimer::RunL()\n");
	
	TSRMLS_FETCH();

	if(iStatus == KErrNone) {
		EG(timed_out) = 1;  // This will be checked in PHP execution loop
	}
	// What to do on errors?  Resubmit (with elapsed time subtracted?)?
	// Signal php error?
}

void CTimeoutTimer::DoCancel() {
	php_s60_log("entering CTimeoutTimer::DoCancel()\n");
	iTimer.Cancel();
}

class CTimeoutMessageHandler : public CActive {
public:
	static CTimeoutMessageHandler* NewL(CActiveScheduler& aActiveScheduler,
	                                    CTimeoutTimer& aTimeoutTimer);
	static CTimeoutMessageHandler* NewLC(CActiveScheduler& aActiveScheduler,
	                                     CTimeoutTimer& aTimeoutTimer);
	virtual ~CTimeoutMessageHandler();
	
	void WaitForMessage();

protected:
	void RunL();
	void DoCancel();

private:
	// Private Symbian second phase constructor
	void ConstructL();
	// Private C++ constructor
	CTimeoutMessageHandler(CActiveScheduler& aActiveScheduler,
	                       CTimeoutTimer& aTimeoutTimer);
	// Copy constructor and assignment operator not implemented
	CTimeoutMessageHandler(const CTimeoutMessageHandler& other);
	CTimeoutMessageHandler& operator=(const CTimeoutMessageHandler& other);

	CActiveScheduler& iActiveScheduler;
	CTimeoutTimer& iTimeoutTimer;
};

CTimeoutMessageHandler* CTimeoutMessageHandler::NewL(CActiveScheduler& aActiveScheduler,
                                                     CTimeoutTimer& aTimeoutTimer) {
	CTimeoutMessageHandler* result = CTimeoutMessageHandler::NewLC(aActiveScheduler,
	                                                               aTimeoutTimer);
	CleanupStack::Pop(result);
	return result;
}

CTimeoutMessageHandler* CTimeoutMessageHandler::NewLC(CActiveScheduler& aActiveScheduler,
                                                      CTimeoutTimer& aTimeoutTimer) {
	CTimeoutMessageHandler* result = new (ELeave) CTimeoutMessageHandler(aActiveScheduler,
	                                                                     aTimeoutTimer);
	CleanupStack::PushL(result);
	result->ConstructL();
	return result;
}

CTimeoutMessageHandler::CTimeoutMessageHandler(CActiveScheduler& aActiveScheduler,
                                               CTimeoutTimer& aTimeoutTimer)
	: CActive(EPriorityStandard),
      iActiveScheduler(aActiveScheduler),
	  iTimeoutTimer(aTimeoutTimer)
{
	CActiveScheduler::Add(this);
}

void CTimeoutMessageHandler::ConstructL() {
}

CTimeoutMessageHandler::~CTimeoutMessageHandler() {
	Cancel();
}

void CTimeoutMessageHandler::WaitForMessage() {
	php_s60_log("entering CTimeoutMessageHandler::WaitForMessage()\n");
	timeoutController->RegisterMessageHandler(&iStatus);
	iStatus = KRequestPending;
	SetActive();
	timeoutController->SignalSemaphore();
}

void CTimeoutMessageHandler::RunL() {
	php_s60_log("entering CTimeoutMessageHandler::RunL()\n");
	switch(iStatus.Int()) {
	case KTimeoutSignalSetTimeout:
		iTimeoutTimer.RequestTimeout(timeoutController->MessageParameter());
		break;
	case KTimeoutSignalCancelTimeout:
		iTimeoutTimer.Cancel();
		break;
	case KTimeoutSignalKillTimeoutThread:
		iActiveScheduler.Stop();
		break;
	}
	WaitForMessage();
}

void CTimeoutMessageHandler::DoCancel() {
	// This should not be called?
}

static TInt ZendTimeoutSymbianThreadProc(TAny*) {
	php_s60_log("entering ZendTimeoutSymbianThreadProc()\n");
	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	
	TSRMLS_FETCH();

	if(cleanupStack == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error creating cleanup stack for timeout thread.");
		return 0;
	}
	php_s60_log("created cleanupStack in ZendTimeoutSymbianThreadProc()\n");
	TRAPD(error, {
		CActiveScheduler* activeScheduler = new (ELeave) CActiveScheduler();
		CActiveScheduler::Install(activeScheduler);
		CleanupStack::PushL(activeScheduler);
		CTimeoutTimer* timer = CTimeoutTimer::NewLC();
		CTimeoutMessageHandler* messageHandler = CTimeoutMessageHandler::NewLC(*activeScheduler, *timer);
		messageHandler->WaitForMessage();
		php_s60_log("about to call activeScheduler->Start() in ZendTimeoutSymbianThreadProc()\n");
		activeScheduler->Start();
		php_s60_log("returned from activeScheduler->Start() in ZendTimeoutSymbianThreadProc()\n");
		CleanupStack::PopAndDestroy(3, activeScheduler);
	});
	delete cleanupStack;
	if(error != KErrNone) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error in timeout thread.  Symbian leave code: %d.",
		                 error);
		
	}
	return 0;
}

_LIT(KTimeoutThreadName, "Zend Timeout Thread");

static void start_symbian_timeout_thread() {
	php_s60_log("entered start_symbian_timeout_thread()\n");
	TRAPD(error, timeoutController = CZendTimeoutSymbianController::NewL());
	
	TSRMLS_FETCH();

	if(error != KErrNone) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error preparing timeout thread creation.  Symbian error code: %d.",
		                 error);
	}
	error = timeout_thread.Create( KTimeoutThreadName,
	                               ZendTimeoutSymbianThreadProc,
	                               0x2000, NULL, NULL );
	if (error != KErrNone) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error creating timeout thread.  Symbian error code: %d.",
		                 error);
	}
	timeoutThreadActive = ETrue;
	php_s60_log("about to resume timeout_thread in start_symbian_timeout_thread()\n");
	timeout_thread.Resume();
}

void zend_set_timeout_symbian(long seconds) {
	/* If support for multiple PHP threads is added,
	   this needs to be made thread safe too. */
	php_s60_log("entered zend_set_timeout_symbian()\n");
	if(!timeoutThreadActive) {
		start_symbian_timeout_thread();
	}
	timeoutController->SetTimeout(seconds);
}

void zend_unset_timeout_symbian() {
	if(timeoutThreadActive) {
		timeoutController->CancelTimeout();
	}
}

// Where does the timeout thread get killed?  Does it need to be?


