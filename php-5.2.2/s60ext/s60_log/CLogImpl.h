#ifndef __CLOGIMPL_H__
#define __CLOGIMPL_H__

#include <e32base.h>		// CActive
#include <F32FILE.H>
#include <LOGVIEW.H>
#include <logcli.h>
#include "s60ext_tools.h"

class CWaitAndReturn : public CActive 
{
public:
	CWaitAndReturn();
	~CWaitAndReturn();
	
	TInt Wait();

private:
	void RunL();
	void DoCancel();
	
private:
	CActiveSchedulerWait* iWait;
};
	
class CLogImpl : public CRefcounted
{
public:
	static CLogImpl* NewL();
	~CLogImpl();
	
	const CLogEvent& FirstEntryL();
	const CLogEvent& NextEntryL();
	const CLogEvent& LastEntryL();
	const CLogEvent& PrevEntryL();
	
	void SetFilterL();

private:
	CLogImpl();
	void ConstructL();
	

	
private:

	CLogClient*			iLogClient; 
	CLogViewEvent*	iLogView;
	CLogFilter*			iLogFilter;
	RFs							iFsSession;	
};

#endif
