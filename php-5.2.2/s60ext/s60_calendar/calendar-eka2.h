// EKA2 implementation using Calendar Interim API
#include <calalarm.h>
#include <calsession.h>
#include <calinstance.h>
#include <calinstanceview.h>
#include <calentry.h>
#include <calprogresscallback.h>
#include <calrrule.h>
#include <caliterator.h>


// Filters.
#define APPTS_INC_FILTER       0x01
#define EVENTS_INC_FILTER      0x02
#define ANNIVS_INC_FILTER      0x04
#define TODOS_INC_FILTER       0x08
#define REMINDERS_INC_FILTER   0x10
#define TODOS_COMPL_INC_FILTER      0x20
#define TODOS_INCOMPL_INC_FILTER    0x40


// This is the time set for entries that repeat forever
#define REPEAT_FOREVER_TIME 4133894400.0

/*
 * Checks if the value represents a weeknumber (in month).
 */
static TBool isWeekInMonth(TInt value)
{
	switch(value){
	case 1: // first week in month
	case 2:
	case 3:
	case 4:
	case -1: //last week in month
		return ETrue;
	}
	return EFalse;
}


/****************************************************************************
 ****************************************************************************/

class CProgress : public CBase,
                  public MCalProgressCallBack
    {
public:
    ~CProgress()
        {
        delete ipScheduler;
        }

    static CProgress* NewL()
        {
        CProgress
            *pThis = NewLC();
        
        CleanupStack::Pop(pThis);

        return pThis;
        }

    static CProgress* NewLC()
        {
        CProgress
            *pThis = new (ELeave) CProgress;
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL();

        return pThis;
        }

    
    void Wait()
        {
        ipScheduler->Start();
        }

private:
    void ConstructL()
        {
        ipScheduler = new (ELeave) CActiveSchedulerWait;
        }
    
    void Progress(TInt /*aPercentageCompleted*/)
        {
        }


    void Completed(TInt /*aError*/)
        {
        ipScheduler->AsyncStop();
        }

    TBool NotifyProgress()
        {
        return EFalse;
        }

private:
    CActiveSchedulerWait* ipScheduler;
    };

 
 


class CalendarDbHandle : public Refcounted {
	~CalendarDbHandle() {
		delete iIView;
		delete iEView;
		delete iCallback;
		delete iSession;
	}
	
	CalendarDbHandle() : iSession(0), iIView(0), iEView(0), iCallback(0) {}
	
public:
    CCalSession *iSession;
    CCalInstanceView *iIView;
    CCalEntryView *iEView;
    CProgress *iCallback;

   	static CalendarDbHandle* NewL(
		char* filename, int filename_len,
		char* flag_str, int flag_str_len)
	{
		char flag = ParseFlagL(flag_str, flag_str_len);
		CCalSession *pSession = CCalSession::NewL();
		CleanupStack::PushL(pSession);
		if (!filename) {
			pSession->OpenL(KNullDesC);
		} else {
			TBuf<KMaxFileName> filenameDes;
			TInt error = convert_c_string_to_unicode(filenameDes, filename, filename_len);
			User::LeaveIfError(error);
			switch (flag) {
			case 'e':
				pSession->OpenL(filenameDes);
			break;
			case 'c':
				// Open, create if file doesn't exist.
				TRAP(error, pSession->OpenL(filenameDes));
				if (error != KErrNone) {
					pSession->CreateCalFileL(filenameDes);
					pSession->OpenL(filenameDes);
				}
			break;
			case 'n':
				// Create a new empty file
				TRAP(error, pSession->DeleteCalFileL(filenameDes));
				pSession->CreateCalFileL(filenameDes);
				pSession->OpenL(filenameDes);
			break;	
			}
		}
		CProgress *pCallback = CProgress::NewLC();
		CCalInstanceView *pIView = CCalInstanceView::NewL(*pSession, *pCallback);
		CleanupStack::PushL(pIView);
		pCallback->Wait();
		
		CCalEntryView *pEView = CCalEntryView::NewL(*pSession, *pCallback);
		CleanupStack::PushL(pEView);
		pCallback->Wait();
		
		CalendarDbHandle* dbHandle = new (ELeave) CalendarDbHandle;
		
		CleanupStack::Pop(pIView);
		CleanupStack::Pop(pEView);
		CleanupStack::Pop(pCallback);
		CleanupStack::Pop(pSession);
		
		dbHandle->iSession = pSession;
		dbHandle->iCallback = pCallback;
		dbHandle->iIView = pIView;
		dbHandle->iEView = pEView;
		return dbHandle;
	}
	
	void GetEntriesL(zval* return_value, zval* zUniqueId);
	
	void GetTodoListsL(zval* return_value) {
		LEAVE_ERROR("Not supported");
	}
	
	void FindInstancesMonthL(zval* return_value, double month, long filterInt);
	
	void FindInstancesDayL(zval* return_value, double day, long filterInt);
	
	void FindInstancesRangeL(zval* return_value, double start, double end, char* search, int search_len, long filterInt);
}; 


TBool IsNullTime(const TTime& theTime)
{
	return theTime == Time::NullTTime();
}





/****************************************************************************
 ****************************************************************************/

 
 


class CalendarEntry : public Refcounted {
	CalendarDbHandle* dbHandle;
	CCalEntry* iEntry;
	CalendarInstance* iInstance; // instance may own this entry
	bool iNew;
	
	~CalendarEntry();
	
public:
	CalendarEntry() : dbHandle(0), iEntry(0), iInstance(0), iNew(false) {
	}
	
	void ConstructNewL(CalendarDbHandle* dbHandle, long type) {
		static TInt s_counter = 0;
		HBufC8* guid = HBufC8::NewLC(256);
		_LIT8(KGuidFormat, "php-%d-%d");
		guid->Des().Format(KGuidFormat, time(NULL), s_counter++); // XXX kinda stupid id generation
		CCalEntry* entry = CCalEntry::NewL(static_cast<CCalEntry::TType>(type), guid, CCalEntry::EMethodNone, 0);
		CleanupStack::Pop(guid); //guid is now owned by entry
		iEntry = entry;
		this->dbHandle = dbHandle;
		dbHandle->incRef();
		iNew = true;
	}
	
	// common interface
	void ConstructExistingL(CalendarDbHandle* dbHandle, zval* zUniqueId) {
		convert_to_string(zUniqueId);
		
		TPtr8 uniqueIdPtr((TUint8*)Z_STRVAL_P(zUniqueId), Z_STRLEN_P(zUniqueId), Z_STRLEN_P(zUniqueId));
		RPointerArray< CCalEntry > entries;
		CleanupClosePushL(entries);

		dbHandle->iEView->FetchL(uniqueIdPtr, entries);
		
		if (entries.Count() == 0) {
			LEAVE_ERROR("Entry not found");	
		}
		ConstructExistingL(dbHandle, entries[0]);
		CleanupStack::PopAndDestroy(); // entries
	}
	
	// EKA2-only interface
	void ConstructExistingL(CalendarDbHandle* dbHandle, CCalEntry* aEntry) {
		iEntry = aEntry;
		this->dbHandle = dbHandle;
		dbHandle->incRef();
		iNew = false;
	}
	
	/**
	 * Construct an entry that is owned by an instance
	 */
	void ConstructExistingL(CalendarDbHandle* dbHandle, CalendarInstance* aInstance);
	
	void RemoveL() {
		// Instance objects disappear?
		dbHandle->iEView->DeleteL(*iEntry);
	}
	
	void CommitL() {
		RPointerArray< CCalEntry > entries;
		CleanupClosePushL(entries);
		
		entries.AppendL(iEntry);
		
		TRAPD(error, {
			TInt success = 0;
			if (iNew) {
				dbHandle->iEView->StoreL(entries, success);
				iNew = false;
			} else {
				dbHandle->iEView->UpdateL(entries, success);
			}
			if (success != 1) {
				LEAVE_ERROR("Entry could not be committed");	
			}
		});
		if (error == KErrCorrupt) {
			LEAVE_ERROR("Entry incomplete or broken; could not be committed");	
		}
		User::LeaveIfError(error);
		
		CleanupStack::PopAndDestroy(); // entries
	}
	
	void GetIdL(zval* return_value) {
		RETURN_STRINGL((char*)iEntry->UidL().Ptr(), iEntry->UidL().Size(), 1);
	}
	
	void GetTypeL(zval* return_value) {
		RETURN_LONG(iEntry->EntryTypeL());
	}
	
	void GetLastModifiedL(zval* return_value) {
		RETURN_DOUBLE(convert_utc_time_to_utc_real(iEntry->LastModifiedDateL().TimeUtcL()));
	}
	
	void GetContentL(zval* return_value) {
		// XXX should use description or summary?
		// At least summary shows up ok in calendar application,
		// but then what is description for?
		TInt error = ZVAL_SYMBIAN_STRING(return_value, iEntry->SummaryL());
		User::LeaveIfError(error);
	}
	
	void SetContentL(char* content, int content_len) {
		HBufC* contentBuf = convert_c_string_to_unicodeLC(content, content_len);
		iEntry->SetSummaryL(*contentBuf);
		CleanupStack::PopAndDestroy(contentBuf);
	}

	void GetLocationL(zval* return_value) {
		TInt error = ZVAL_SYMBIAN_STRING(return_value, iEntry->LocationL());
		User::LeaveIfError(error);
	}

	void SetLocationL(char* location, int location_len) {
		HBufC* locationBuf = convert_c_string_to_unicodeLC(location, location_len);
		iEntry->SetLocationL(*locationBuf);
		CleanupStack::PopAndDestroy(locationBuf);
	}
	
	void GetReplicationL(zval* return_value) {
		RETURN_LONG(iEntry->ReplicationStatusL());
	}
	
	void SetReplicationL(long repl) {
		iEntry->SetReplicationStatusL(static_cast<CCalEntry::TReplicationStatus>(repl));
	}
	
	void GetStartTimeL(zval* return_value) {
		TTime time = iEntry->StartTimeL().TimeUtcL();
		if (IsNullTime(time)) {
			RETURN_FALSE;	
		}
		RETURN_DOUBLE(convert_utc_time_to_utc_real(time));
	}
	
	void GetEndTimeL(zval* return_value) {
		TTime time = iEntry->EndTimeL().TimeUtcL();
		if (IsNullTime(time)) {
			RETURN_FALSE;	
		}
		RETURN_DOUBLE(convert_utc_time_to_utc_real(time));
	}
	
	void SetStartAndEndTimeL(double start, double end) {
		TCalTime startTime;
		TCalTime endTime;
		startTime.SetTimeUtcL(convert_utc_real_to_utc_time(start));
		endTime.SetTimeUtcL(convert_utc_real_to_utc_time(end));
		iEntry->SetStartAndEndTimeL(startTime, endTime);
	}

	void GetAlarmL(zval* return_value) {
		CCalAlarm * alarm = iEntry->AlarmL();
		if (!alarm) { 
			RETURN_FALSE;
		}
		TTimeIntervalMinutes t = alarm->TimeOffset();
		delete alarm;
		TTime eventTime = iEntry->StartTimeL().TimeUtcL();
		if (IsNullTime(eventTime)) {
			RETURN_FALSE;	
		}
		RETURN_DOUBLE(convert_utc_time_to_utc_real(eventTime - t));
	}

	void ClearAlarmL() {
		iEntry->SetAlarmL(0);
	}
	
	void SetAlarmL(double alarm) {
		// calculate alarm interval:
		TTime eventTime = iEntry->StartTimeL().TimeUtcL();
		TTime alarmTime = convert_utc_real_to_utc_time(alarm);
		TTimeIntervalMinutes interval;
		User::LeaveIfError(eventTime.MinutesFrom(alarmTime, interval));
		
		// create and set alarm
		CCalAlarm * newAlarm = CCalAlarm::NewL();
		CleanupStack::PushL(newAlarm);
		newAlarm->SetTimeOffset(interval);
		iEntry->SetAlarmL(newAlarm);
		CleanupStack::PopAndDestroy(newAlarm);
	}
	
	void GetCrossedOutL(zval* return_value) {
		TTime t = iEntry->CompletedTimeL().TimeUtcL();
		if (IsNullTime(t)) {
			RETURN_FALSE;	
		}
		RETURN_DOUBLE(convert_utc_time_to_utc_real(t));
	}
	
	void ClearCrossedOutL() {
		iEntry->SetCompletedL(EFalse, TCalTime());
	}
	
	void SetCrossedOutL(double crossed_out) {
		TCalTime t;
		t.SetTimeUtcL(convert_utc_real_to_utc_time(crossed_out));
		iEntry->SetCompletedL(ETrue, t);
	}
	
	void GetTodoListL(zval *return_value) {
		LEAVE_ERROR("Not supported");
	}

	void SetTodoListL(CalendarTodoList* todolist) {
		LEAVE_ERROR("Not supported");
	}
	
	void GetPriorityL(zval *return_value) {
		RETURN_LONG(iEntry->PriorityL());
	}
	
	void SetPriorityL(long priority) {
		iEntry->SetPriorityL(priority);
	}

	void GetRepeatDaysL(zval* zre, TCalRRule& rule);	
	void GetRepeatL(zval* return_value);	
	void ClearRepeatL() {
		iEntry->ClearRepeatingPropertiesL();
	}
	
	void SetRepeatL(TInt type, TReal startDate, TReal endDate,
				TBool eternalRepeat, TInt interval,
				CArrayFixSeg<TReal>* exceptionArray,
				CArrayFixSeg<TReal>* rdateArray, HashTable *repeatDays);

	bool AddCalTimeArrayL(zval* return_value, RArray< TCalTime >& dateList, const char* key) {
		if (dateList.Count() > 0) {
			zval* zex;
			ALLOC_INIT_ZVAL(zex);
			array_init(zex);
			for (TInt i = 0; i < dateList.Count(); i++) {
				TTime time;
				TRAPD(error, time = dateList[i].TimeUtcL());
				if (error != KErrNone) {
					zval_ptr_dtor(&zex);		
					User::Leave(error);	
				}
				add_next_index_double(zex, convert_utc_time_to_utc_real(time));
			}
			add_assoc_zval(return_value, (char*)key, zex);
			return true;
		}
		return false;
	}				
				
	bool AddRepeatExceptionsL(zval* return_value)
	{
		RArray< TCalTime > exDateList;
		CleanupClosePushL(exDateList);
		iEntry->GetExceptionDatesL(exDateList);
		bool addedSome = AddCalTimeArrayL(return_value, exDateList, KExceptions);
		CleanupStack::PopAndDestroy(); //exDateList
		return addedSome;
	}
	
	bool AddRepeatRDatesL(zval* return_value)
	{
		RArray< TCalTime > RDateList;
		CleanupClosePushL(RDateList);
		iEntry->GetRDatesL(RDateList);
		bool addedSome = AddCalTimeArrayL(return_value, RDateList, KRDates);
		CleanupStack::PopAndDestroy(); //RDateList
		return addedSome;
	}
				
				
				
				
				
				
					
	void GetStatusL(zval* return_value) {
		RETURN_LONG(iEntry->StatusL());
	}
	
	void SetStatusL(long status) {
		CCalEntry::TStatus s = static_cast<CCalEntry::TStatus>(status);
		if (iEntry->EntryTypeL() == CCalEntry::ETodo) {
			if (s != CCalEntry::ECancelled && 
			    s != CCalEntry::ETodoNeedsAction && 
			    s != CCalEntry::ETodoCompleted && 
			    s != CCalEntry::ETodoInProcess)
			    LEAVE_ERROR("Bad status for TODO");
			iEntry->SetStatusL(s);
		} else {
			if (s != CCalEntry::ECancelled && 
			    s != CCalEntry::ETentative && 
			    s != CCalEntry::EConfirmed)
			    LEAVE_ERROR("Bad status");
			iEntry->SetStatusL(s);
		}
	}
};





/****************************************************************************
 ****************************************************************************/

 
 
 


class CalendarInstance : public Refcounted {
	CalendarDbHandle* dbHandle;
	
	~CalendarInstance() {
		delete iInstance; //XXX ok? not ok?
		SAFE_CLEAR_REFERENCE(dbHandle);
	}
public:
	CCalInstance* iInstance;
	
	CalendarInstance() : dbHandle(0), iInstance(0) {
	}
	
	void ConstructL(CalendarDbHandle* dbHandle, CCalInstance* aInstance) {
		this->dbHandle = dbHandle;
		dbHandle->incRef();
		iInstance = aInstance;
	}
	
	void GetEntryL(zval* return_value) {
		TSRMLS_FETCH(); 
		
		if (object_init_ex(return_value, CLASS_ENTRY(s60_calendar_Entry)) == SUCCESS) {
			GET_STRUCT(return_value, s60_calendar_Entry)->entry->ConstructExistingL(dbHandle, this);
		} else {
			User::Leave(KErrGeneral);	
		}
	}
	
	void GetDateL(zval* return_value) {
		RETURN_DOUBLE(convert_utc_time_to_utc_real(iInstance->Time().TimeUtcL()));
	}
	
	void RemoveL(long which) {
		CalCommon::TRecurrenceRange eWhich;
		if (which == REMOVE_THIS) {
			eWhich = CalCommon::EThisOnly;
		} else if (which == REMOVE_FUTURE) {
			eWhich = CalCommon::EThisAndFuture;
		} else {
			eWhich = CalCommon::EThisAndPrior;
		}
		dbHandle->iIView->DeleteL(iInstance, eWhich);
		iInstance = 0; // XXX apparently DeleteL already deletes iInstance
	}
};





/****************************************************************************
 ****************************************************************************/

 
 
 


class CalendarTodoList : public Refcounted {
	CalendarDbHandle* dbHandle;
	
	~CalendarTodoList() {
		SAFE_CLEAR_REFERENCE(dbHandle);
	}
public:
	CalendarTodoList() : dbHandle(0) {
	}
	
	void ConstructNewL(CalendarDbHandle* dbHandle) {
		LEAVE_ERROR("Not supported");
	}
	
	void ConstructExistingL(CalendarDbHandle* dbHandle, long uniqueId) {
		LEAVE_ERROR("Not supported");
	}
	
	void GetTodosL(zval* return_value) {
		LEAVE_ERROR("Not supported");
	}		
	
	void GetIdL(zval* return_value) {
		LEAVE_ERROR("Not supported");
	}
	
	void GetNameL(zval* return_value) {
		LEAVE_ERROR("Not supported");
	}
	
	void SetNameL(char* name, int name_len) {
		LEAVE_ERROR("Not supported");
	}
	
	void RemoveL() {
		LEAVE_ERROR("Not supported");
	}
};





/****************************************************************************
 ****************************************************************************/

 
 
 
static void register_constants(INIT_FUNC_ARGS) {
    // Entry types. 
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_MEETING", CCalEntry::EAppt, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_EVENT", CCalEntry::EEvent, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_ANNIV", CCalEntry::EAnniv, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_TODO", CCalEntry::ETodo, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_REMINDER", CCalEntry::EReminder, CONST_CS | CONST_PERSISTENT);

	// Filters
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_MEETING", APPTS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_EVENT", EVENTS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_ANNIV", ANNIVS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_TODO", TODOS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_REMINDER", REMINDERS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	//REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_TODO_COMPLETED", TODOS_COMPL_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	//REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_TODO_INCOMPLETED", TODOS_INCOMPL_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	
    // Replication statuses.
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REP_OPEN", CCalEntry::EOpen, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REP_PRIVATE", CCalEntry::EPrivate, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REP_RESTRICTED", CCalEntry::ERestricted, CONST_CS | CONST_PERSISTENT);
	
    // Statuses.
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_CANCELLED", CCalEntry::ECancelled, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_TODO_NEED_ACTION", CCalEntry::ETodoNeedsAction, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_TODO_COMPLETED", CCalEntry::ETodoCompleted, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_TODO_IN_PROGRESS", CCalEntry::ETodoInProcess, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_TENTATIVE", CCalEntry::ETentative, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_CONFIRMED", CCalEntry::EConfirmed, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_NULL", CCalEntry::ENullStatus, CONST_CS | CONST_PERSISTENT);
	
	// Remove instance modifiers.
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REMOVE_THIS", REMOVE_THIS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REMOVE_FUTURE", REMOVE_FUTURE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REMOVE_PAST", REMOVE_PAST, CONST_CS | CONST_PERSISTENT);
}

void  CalendarDbHandle::FindInstancesMonthL(zval* return_value, double month, long filterInt)
{
	TDateTime aDateTime = convert_utc_real_to_time(month).DateTime();
	aDateTime.SetDay(0); 
	aDateTime.SetHour(0);
	aDateTime.SetMinute(0);
	aDateTime.SetSecond(0);
	TTime start(aDateTime);
	TTime end = start + TTimeIntervalMonths(1);
	FindInstancesRangeL(return_value,
		convert_time_to_utc_real(start),
		convert_time_to_utc_real(end)-1,
		(char*)"", 0, filterInt);
}


/*
 * Returns date value to corresponds the given datetime value.
 */
TTime truncateToDate(const TTime& theTime)
{
	TDateTime aDateTime = theTime.DateTime();
	aDateTime.SetHour(0); 
	aDateTime.SetMinute(0); 
	aDateTime.SetSecond(0); 
	return TTime(aDateTime);
}


void CalendarDbHandle::FindInstancesDayL(zval* return_value, double day, long filterInt)
{
	TTime start = truncateToDate(convert_utc_real_to_time(day));
	TTime end = start + TTimeIntervalDays(1);
	FindInstancesRangeL(return_value,
		convert_time_to_utc_real(start),
		convert_time_to_utc_real(end)-1,
		(char*)"", 0, filterInt);
}


void CalendarDbHandle::FindInstancesRangeL(zval* return_value, double start, double end, char* search, int search_len, long filterInt)
{
	TSRMLS_FETCH(); 
	
	RPointerArray< CCalInstance > matchedInstanceList;
	CleanupClosePushL(matchedInstanceList);
	CalCommon::TCalViewFilter calViewFilter = 0;
	if (filterInt == 0) {
		calViewFilter = CalCommon::EIncludeAll;
	} else {
		if (filterInt & APPTS_INC_FILTER) calViewFilter |= CalCommon::EIncludeAppts;
		if (filterInt & EVENTS_INC_FILTER) calViewFilter |= CalCommon::EIncludeEvents;
		if (filterInt & ANNIVS_INC_FILTER) calViewFilter |= CalCommon::EIncludeAnnivs;
		if (filterInt & TODOS_INC_FILTER) calViewFilter |= CalCommon::EIncludeCompletedTodos | CalCommon::EIncludeIncompletedTodos;
		if (filterInt & REMINDERS_INC_FILTER) calViewFilter |= CalCommon::EIncludeReminder;
		if (filterInt & TODOS_COMPL_INC_FILTER) calViewFilter |= CalCommon::EIncludeCompletedTodos;
		if (filterInt & TODOS_INCOMPL_INC_FILTER) calViewFilter |= CalCommon::EIncludeIncompletedTodos;
	}
	
	TCalTime startTime;
	TCalTime endTime;
	startTime.SetTimeUtcL(convert_utc_real_to_utc_time(start));
	endTime.SetTimeUtcL(convert_utc_real_to_utc_time(end));
	CalCommon::TCalTimeRange timeRange(startTime, endTime);
	HBufC* searchBuf = convert_c_string_to_unicodeLC(search, search_len);
	CCalInstanceView::TCalSearchParams searchParams(*searchBuf, CalCommon::EExactTextSearch);
	CleanupStack::PopAndDestroy(searchBuf);
	
	iIView->FindInstanceL(matchedInstanceList, calViewFilter, timeRange, searchParams);
	
	array_init(return_value);
	for (int i = 0; i < matchedInstanceList.Count(); ++i) {
		CCalInstance* inst = matchedInstanceList[i];
		
		// add instance to list
		zval *new_object;
		ALLOC_INIT_ZVAL(new_object);
		// create php object
		if (object_init_ex(new_object, CLASS_ENTRY(s60_calendar_Instance)) != SUCCESS) {
			User::Leave(KErrGeneral);
		}
		TRAPD(error, GET_STRUCT(new_object, s60_calendar_Instance)->instance->ConstructL(this, inst));
		if (error != KErrNone) {
			zval_ptr_dtor(&new_object);			
			User::Leave(error);	
		}
		// append object to list
		add_next_index_zval(return_value, new_object);
	}
	CleanupStack::PopAndDestroy(); // matchedInstanceList
}


 
CalendarEntry::~CalendarEntry() {
	if (iInstance) {
		SAFE_CLEAR_REFERENCE(iInstance);
	} else {
		//delete iEntry;
	}
	SAFE_CLEAR_REFERENCE(dbHandle);
}	

void CalendarEntry::ConstructExistingL(CalendarDbHandle* dbHandle, CalendarInstance* aInstance) {
	iInstance = aInstance;
	iInstance->incRef();
	this->dbHandle = dbHandle;
	dbHandle->incRef();
	
	iEntry = &aInstance->iInstance->Entry();
	iNew = false;
}


// adds all entries with id "uniqueId" to array "return_value"
static void AddEntriesL(CalendarDbHandle* dbHandle, zval* return_value, const TDesC8& uniqueId) {
	TSRMLS_FETCH(); 

	RPointerArray< CCalEntry > entries;
	CleanupClosePushL(entries);
	dbHandle->iEView->FetchL(uniqueId, entries);
	
	for (int i = 0; i < entries.Count(); ++i) {
		CCalEntry* entry = entries[i];
		
		// add entry to list
		zval *new_object;
		ALLOC_INIT_ZVAL(new_object);
		// create php object
		if (object_init_ex(new_object, CLASS_ENTRY(s60_calendar_Entry)) != SUCCESS) {
			User::Leave(KErrGeneral);
		}
		TRAPD(error, GET_STRUCT(new_object, s60_calendar_Entry)->entry->ConstructExistingL(dbHandle, entry));
		if (error != KErrNone) {
			zval_ptr_dtor(&new_object);			
			User::Leave(error);	
		}
		// append object to list
		add_next_index_zval(return_value, new_object);
	}
	CleanupStack::PopAndDestroy(); // entries
}


void CalendarDbHandle::GetEntriesL(zval* return_value, zval* zUniqueId) {
	if (zUniqueId == NULL) {
		CCalIter* iter = CCalIter::NewL(*iSession);
		CleanupStack::PushL(iter);
		const TDesC8& d = iter->FirstL();
		array_init(return_value);
		if (d != KNullDesC8) {
			AddEntriesL(this, return_value, d);
			while (1) {
				const TDesC8& d2 = iter->NextL();
				if (d2 == KNullDesC8) break;
				AddEntriesL(this, return_value, d2);
			}
		}
		CleanupStack::PopAndDestroy(iter);
	} else {
		convert_to_string(zUniqueId);
		
		TPtr8 uniqueIdPtr((TUint8*)Z_STRVAL_P(zUniqueId), Z_STRLEN_P(zUniqueId), Z_STRLEN_P(zUniqueId));
		array_init(return_value);
		AddEntriesL(this, return_value, uniqueIdPtr);
	}
}

void CalendarEntry::GetRepeatDaysL(zval* zre, TCalRRule& rule)
{
	switch (rule.Type()) {
	case TCalRRule::EWeekly:
	{        
		RArray< TDay > days;
		CleanupClosePushL(days);
		rule.GetByDayL(days);
		for(TInt i = 0; i < days.Count(); i++) {
			add_next_index_long(zre, (long)days[i]);
		}
		CleanupStack::PopAndDestroy(); //days
	}  
	break;
	case TCalRRule::EMonthly:
	{
		// XXX I don't know if there could be both
		// days and dates information at the same time,
		// so both are appended here to the array.
		// Just so users get the information even if it's
		// a bit unexpected..
		
		// (monthly_by_days)
		RArray< TCalRRule::TDayOfMonth > days;
		CleanupClosePushL(days);
		rule.GetByDayL(days);
		for(TInt i = 0; i < days.Count(); i++) {
			TCalRRule::TDayOfMonth dm = days[i];
			
			zval* zsub;
			ALLOC_INIT_ZVAL(zsub);
			array_init(zsub);	
			add_assoc_long(zsub, KDay, (long)dm.Day());
			add_assoc_long(zsub, KWeek, dm.WeekInMonth());
			add_next_index_zval(zre, zsub);
		}
		CleanupStack::PopAndDestroy(); //days
	}
	{
		// (monthly_by_dates)
		RArray< TInt > monthDays;
		CleanupClosePushL(monthDays);
		rule.GetByMonthDayL(monthDays);
		for(TInt i = 0; i < monthDays.Count(); i++) {
			add_next_index_long(zre, monthDays[i]);
		}
		CleanupStack::PopAndDestroy(); //monthDays
	}
	break;
	case TCalRRule::EYearly:
	{
		RArray< TCalRRule::TDayOfMonth > days;
		CleanupClosePushL(days);
		rule.GetByDayL(days);
		if (days.Count() == 0) {
			// yearly_by_dates repeat
			CleanupStack::PopAndDestroy(); //days
			break;
		}
		TCalRRule::TDayOfMonth dm = days[0];
		
		add_assoc_long(zre, KDay, (long)dm.Day());
		add_assoc_long(zre, KWeek, dm.WeekInMonth());
		
		CleanupStack::PopAndDestroy(); //days
		
		RArray< TMonth > months;
		CleanupClosePushL(months);
		rule.GetByMonthL(months);
		if (months.Count() != 1) {
			LEAVE_ERROR("Bad yearly rule");	
		}
		
		add_assoc_long(zre, KMonth, (long)months[0]);
		CleanupStack::PopAndDestroy(); //months
	}
	break;
    case TCalRRule::EDaily:
    case TCalRRule::EInvalid:
    break;
	}
}

void CalendarEntry::GetRepeatL(zval* return_value)
{
	TCalRRule rule;
	// Special handling if no repeat rule
	// (may return rdates anyway)
	if (!iEntry->GetRRuleL(rule)) {
		array_init(return_value);
		// "type"
		const TDesC8* typePtr = &KNoRepeat;
		add_assoc_string(return_value, KRepeatType, (char*)typePtr->Ptr(), 1);
		bool addedSome = false;
		// "rdates"
		addedSome |= AddRepeatRDatesL(return_value);
		// "exceptions"
		addedSome |= AddRepeatExceptionsL(return_value);
		if (addedSome) {
			return;	
		} else {
			zval_dtor(return_value);
			RETURN_FALSE;
		}
	}
	
	// Fill up the return array
	array_init(return_value);
	// "type"
	const TDesC8* typePtr;
	switch (rule.Type()) {
	case TCalRRule::EDaily: typePtr = &KDaily; break;	
	case TCalRRule::EWeekly: typePtr = &KWeekly; break;	
	case TCalRRule::EMonthly:
	{
		// determine if the rule is monthly_by_dates or monthly_by_days
		RArray< TCalRRule::TDayOfMonth > days;
		CleanupClosePushL(days);
		rule.GetByDayL(days);
		if (days.Count()) {
			typePtr = &KMonthlyByDays;
		} else {
			typePtr = &KMonthlyByDates;
		}
		CleanupStack::PopAndDestroy(); //days;
	}	
	break;	
	case TCalRRule::EYearly:
	{
		// determine if the rule is yearly_by_date or yearly_by_day
		RArray< TCalRRule::TDayOfMonth > days;
		CleanupClosePushL(days);
		rule.GetByDayL(days);
		if (days.Count()) {
			typePtr = &KYearlyByDay;
		} else {
			typePtr = &KYearlyByDate;
		}
		CleanupStack::PopAndDestroy(); //days
	}
	break;	
	case TCalRRule::EInvalid:
	default: typePtr = &KUnknownRep; break;	
	}
	add_assoc_string(return_value, KRepeatType, (char*)typePtr->Ptr(), 1);

	// "start"
	TTime startTime = rule.DtStart().TimeUtcL();
	if (!IsNullTime(startTime)) {
		add_assoc_double(return_value, KStartDate, convert_utc_time_to_utc_real(startTime));
	}
	// "end"
	TTime endTime = rule.Until().TimeUtcL();
	// XXX since (endTime == year 2100) when left unspecified is
	// undefined behaviour, maybe I shouldn't trust on it here.
	if (!IsNullTime(endTime) && convert_utc_time_to_utc_real(endTime) < REPEAT_FOREVER_TIME-3600.0*24.0) {
		add_assoc_double(return_value, KEndDate, convert_utc_time_to_utc_real(endTime));
	}
	// "interval"
	add_assoc_long(return_value, KInterval, rule.Interval());
	
	// "days"
	zval* zre;
	ALLOC_INIT_ZVAL(zre);
	array_init(zre);	
	TRAPD(error, GetRepeatDaysL(zre, rule));
	if (zend_hash_num_elements(Z_ARRVAL_P(zre)) == 0 || error != KErrNone) {
		zval_ptr_dtor(&zre);
		User::LeaveIfError(error);
	} else {
		add_assoc_zval(return_value, KRepeatDays, zre);
	}
	
	// "rdates"
	AddRepeatRDatesL(return_value);
	// "exceptions"
	AddRepeatExceptionsL(return_value);
}


void CalendarEntry::SetRepeatL(TInt type, TReal startDate, TReal endDate,
				TBool eternalRepeat, TInt interval,
				CArrayFixSeg<TReal>* exceptionArray, 
				CArrayFixSeg<TReal>* rdateArray, HashTable *repeatDays)
{
	TCalRRule::TType realType;
	switch(type) {
	case NO_REPEAT:
		break;
	case DAILY_REPEAT:
		realType = TCalRRule::EDaily;
		break;
	case WEEKLY_REPEAT:
		realType = TCalRRule::EWeekly;
		break;
	case MONTHLY_BY_DATES_REPEAT:
	case MONTHLY_BY_DAYS_REPEAT:
		realType = TCalRRule::EMonthly;
		break;
	case YEARLY_BY_DATE_REPEAT:
	case YEARLY_BY_DAY_REPEAT:
		realType = TCalRRule::EYearly;
		break;
	default:
		LEAVE_ERROR("Illegal repeat type");
	}
	
	if (type != NO_REPEAT) {
		TCalRRule rule(realType);
		
		TTime startTTime = convert_utc_real_to_utc_time(startDate);
		TCalTime startTime;
		startTime.SetTimeUtcL(startTTime);
		rule.SetDtStart(startTime);
		
		if (!eternalRepeat) {
			if (endDate < startDate) LEAVE_ERROR("End date before start date");
			TCalTime endTime;
			endTime.SetTimeUtcL(convert_utc_real_to_utc_time(endDate));
			rule.SetUntil(endTime);
		}
		
		rule.SetInterval(interval);
		
		switch(type) {
		case DAILY_REPEAT:
		break;
		case WEEKLY_REPEAT:
		{
			RArray< TDay > days;
			CleanupClosePushL(days);
			
			// Add repeat days.
			if (repeatDays) {
				zval **data;
				ARRAY_FOREACH(pointer, repeatDays, data) {
					if (Z_TYPE_PP(data) != IS_LONG || !isWeekday(Z_LVAL_PP(data))) {
						LEAVE_ERROR("Bad value for weekly repeat day");
					}
					days.Append(static_cast<TDay>(Z_LVAL_PP(data)));
				}
			}
			if (days.Count() == 0) {
				// default repeat day.
				days.Append(startTTime.DayNoInWeek());
			}
			rule.SetByDay(days);
			
			CleanupStack::PopAndDestroy(); //days
		}
		break;
		case MONTHLY_BY_DATES_REPEAT:
		{
			RArray< TInt > monthDays;
			CleanupClosePushL(monthDays);
			// Add repeat dates.
			if (repeatDays) {
				zval **data;
				ARRAY_FOREACH(pointer, repeatDays, data) {
					if (Z_TYPE_PP(data) != IS_LONG || Z_LVAL_PP(data) < 0 || Z_LVAL_PP(data) >= DAYS_IN_MONTH) {
						LEAVE_ERROR("Monthly repeat dates must be integers (0-30)");
					}
					monthDays.Append(Z_LVAL_PP(data));
				}
			}
			if (monthDays.Count() == 0) {
				// default repeat date.
				monthDays.Append(startTTime.DayNoInMonth());
			}
			rule.SetByMonthDay(monthDays);
			CleanupStack::PopAndDestroy(); //monthDays
		}
		break;
		case MONTHLY_BY_DAYS_REPEAT:
		{
			RArray< TCalRRule::TDayOfMonth > days;
			CleanupClosePushL(days);
			
			// Add repeat dates.
			if (repeatDays) {
				zval **data;
				ARRAY_FOREACH(pointer, repeatDays, data) {
					if (Z_TYPE_PP(data) != IS_ARRAY) {
						LEAVE_ERROR("Monthly repeat days must be in arrays");
					}
					zval** zvalue;
					if (zend_hash_find(Z_ARRVAL_PP(data), KWeek, sizeof(KWeek), (void**)&zvalue) != SUCCESS) {
						LEAVE_ERROR("Week is missing from monthly repeat data");
					}
					if (Z_TYPE_PP(zvalue) != IS_LONG || !isWeekInMonth(Z_LVAL_PP(zvalue))) {
						LEAVE_ERROR("Weeks must be integers -1, 1, 2, 3 or 4");	
					}
					long week = Z_LVAL_PP(zvalue);
					
					if (zend_hash_find(Z_ARRVAL_PP(data), KDay, sizeof(KDay), (void**)&zvalue) != SUCCESS) {
						LEAVE_ERROR("Day is missing from monthly repeat data");
					}
					if (Z_TYPE_PP(zvalue) != IS_LONG || !isWeekday(Z_LVAL_PP(zvalue))) {
						LEAVE_ERROR("Days must be integers on range 0-6");	
					}
					long day = Z_LVAL_PP(zvalue);
					days.Append(TCalRRule::TDayOfMonth(static_cast<TDay>(day), week));
				}
			}
			if (days.Count() == 0) {
				// default repeat day.
				days.Append(TCalRRule::TDayOfMonth(startTTime.DayNoInWeek(), startTTime.DayNoInMonth()/DAYS_IN_WEEK));
			}         
			rule.SetByDay(days);
			CleanupStack::PopAndDestroy(); //days
		}
		break;
		case YEARLY_BY_DATE_REPEAT:
		break;
		case YEARLY_BY_DAY_REPEAT:
		{
			long week, day, month;
			if (repeatDays) {
				zval** zvalue;
				if (zend_hash_find(repeatDays, KWeek, sizeof(KWeek), (void**)&zvalue) != SUCCESS) {
					LEAVE_ERROR("Week is missing from yearly repeat data");
				}
				if (Z_TYPE_PP(zvalue) != IS_LONG || !isWeekInMonth(Z_LVAL_PP(zvalue))) {
					LEAVE_ERROR("Weeks must be integers -1, 1, 2, 3 or 4");	
				}
				week = Z_LVAL_PP(zvalue);
				
				if (zend_hash_find(repeatDays, KDay, sizeof(KDay), (void**)&zvalue) != SUCCESS) {
					LEAVE_ERROR("Day is missing from yearly repeat data");
				}
				if (Z_TYPE_PP(zvalue) != IS_LONG || !isWeekday(Z_LVAL_PP(zvalue))) {
					LEAVE_ERROR("Day must be integer on range 0-6");	
				}
				day = Z_LVAL_PP(zvalue);
	
				if (zend_hash_find(repeatDays, KMonth, sizeof(KMonth), (void**)&zvalue) != SUCCESS) {
					LEAVE_ERROR("Month is missing from yearly repeat data");
				}
				if (Z_TYPE_PP(zvalue) != IS_LONG || !isMonth(Z_LVAL_PP(zvalue))) {
					LEAVE_ERROR("Month must be integer on range 0-11");	
				}
				month = Z_LVAL_PP(zvalue);
				//XXX maybe do some checking for day,month,week values?			
			} else {
				// default repeat date.
				day = startTTime.DayNoInWeek();
				week = startTTime.DayNoInMonth()/DAYS_IN_WEEK+1; //XXX this is not really very correct..
				month = startTTime.DateTime().Month();
			}
			RArray< TMonth > months;
			months.Append(static_cast<TMonth>(month));
			RArray< TCalRRule::TDayOfMonth > days;
			days.Append(TCalRRule::TDayOfMonth(static_cast<TDay>(day), week));
			rule.SetByDay(days);
			rule.SetByMonth(months);
			months.Close();
			days.Close();
		}
		break;
		}
		
		iEntry->SetRRuleL(rule);
	} // if (type != NO_REPEAT)

	// exceptions
	if (exceptionArray) {
		RArray< TCalTime > exDateList;
		CleanupClosePushL(exDateList);
		for (TInt i=0; i<exceptionArray->Count(); i++){
			TTime exceptionTime = convert_utc_real_to_utc_time(exceptionArray->At(i));
			TCalTime eCalTime;
			eCalTime.SetTimeUtcL(exceptionTime);
			exDateList.Append(eCalTime);
		}
		iEntry->SetExceptionDatesL(exDateList);
		CleanupStack::PopAndDestroy(); //exDateList
	}

	// rdates
	if (rdateArray) {
		RArray< TCalTime > rDateList;
		CleanupClosePushL(rDateList);
		for (TInt i=0; i<rdateArray->Count(); i++){
			TTime rdateTime = convert_utc_real_to_utc_time(rdateArray->At(i));
			TCalTime eCalTime;
			eCalTime.SetTimeUtcL(rdateTime);
			rDateList.Append(eCalTime);
		}
		iEntry->SetRDatesL(rDateList);
		CleanupStack::PopAndDestroy(); //rDateList
	}
}
