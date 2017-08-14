// EKA1 implementation using Agenda Model API

#include <agclient.h>
#include <agmmodel.h>
#include <agmentry.h>
#include <agmdate.h>
#include <agmalarm.h>
#include <agmrepli.h>

_LIT(KDefaultAgendaFile, "C:\\system\\data\\Calendar");
_LIT(KDefaultTodoListName, "Todo-list");

const TInt KMinsPerHour = 60;

// Filters.
#define APPTS_INC_FILTER   0x01
#define EVENTS_INC_FILTER  0x02
#define ANNIVS_INC_FILTER  0x04
#define TODOS_INC_FILTER   0x08

#define KDefaultTimeForEvents TTimeIntervalMinutes(13 * KMinsPerHour)
#define KDefaultTimeForAnnivs TTimeIntervalMinutes(13 * KMinsPerHour)
#define KDefaultTimeForDayNote  TTimeIntervalMinutes(13 * KMinsPerHour)

#define EARLIEST_ALARM_DAY_INTERVAL  1001
#define LATEST_APPT_ALARM_MINUTE_INTERVAL 23*60+59

#define SERIES60_VERSION 16

#undef HAVE_ALARM
#if SERIES60_VERSION<20
#define HAVE_ALARM
#endif


#define CHECK_AGENDA_STATE_DB_L(dbHandle_) \
	if (dbHandle_->agendaModel->State() != CAgnEntryModel::EOk) { \
		LEAVE_ERROR("Agenda state does not allow the operation"); \
	}

/*
 * Creates new agenda file.
 */
static void createNewAgendaFileL(TDesC& filename)
{
	RFs fileSession;
	User::LeaveIfError(fileSession.Connect());
	CleanupClosePushL(fileSession);
	TFileName aFilename(filename);
	CParaFormatLayer* paraFormatLayer = CParaFormatLayer::NewL();
	CleanupStack::PushL(paraFormatLayer);
	CCharFormatLayer* charFormatLayer = CCharFormatLayer::NewL();
	CleanupStack::PushL(charFormatLayer);
	
	CAgnModel* agendaModel = CAgnModel::NewL(NULL);
	CleanupStack::PushL(agendaModel);
	agendaModel->CreateL(fileSession, 
	               aFilename,
	               KDefaultTodoListName, 
	               paraFormatLayer, 
	               charFormatLayer);

	CleanupStack::PopAndDestroy(agendaModel);
	CleanupStack::PopAndDestroy(charFormatLayer);
	CleanupStack::PopAndDestroy(paraFormatLayer);
	CleanupStack::PopAndDestroy();
}


/*
 * Checks if the time value represents agenda model's null time.
 */
static TBool Is_null_time(const TTime& theTime)
{
	return AgnDateTime::TTimeToAgnDate(theTime)==AgnDateTime::NullDate();
}

/*
 * Checks if the time value is in the limits of agenda model (and not null).
 */
static TBool Check_time_validity(const TTime& theTime)
{
	return !(AgnDateTime::MaxDateAsTTime() < theTime || 
	         AgnDateTime::MinDateAsTTime() > theTime ||
	         AgnDateTime::TTimeToAgnDate(theTime) == AgnDateTime::NullDate());
}


/*
 * Sets the filter to be used in the entry search.
 */
static TAgnFilter makeFilter(TInt filterData)
{
	TAgnFilter filter;
	if (filterData == 0) {
		// 0 = default (include nothing would make no sense)
		return filter;
	}

	// Set the filter data.
	filter.SetIncludeTimedAppts((filterData & APPTS_INC_FILTER) != 0);
	filter.SetIncludeEvents((filterData & EVENTS_INC_FILTER) != 0);
	filter.SetIncludeAnnivs((filterData & ANNIVS_INC_FILTER) != 0);
	filter.SetIncludeTodos((filterData & TODOS_INC_FILTER) != 0);
	return filter;
}


/*
 * Returns date value to corresponds the given datetime value.
 */
static TTime truncateToDate(const TTime& theTime)
{
	TDateTime aDateTime = theTime.DateTime();
	aDateTime.SetHour(0); 
	aDateTime.SetMinute(0); 
	aDateTime.SetSecond(0); 
	return TTime(aDateTime);
}


static void CheckStartAndEndTimeL(const TTime& startTTime, const TTime &endTTime) {
	if (!Check_time_validity(startTTime)) {
		LEAVE_ERROR("Illegal start datetime");
	}
	if (!Check_time_validity(endTTime)) {
		LEAVE_ERROR("Illegal end datetime");
	}
	
	if (startTTime > endTTime) { \
		LEAVE_ERROR("Start datetime cannot be greater than end datetime"); \
	}
}



/*
 * Converts a value -1, 1,2,3,4 to a weeknumber (in month).
 */
static long convertToWeekL(zval** zvalue)
{
	if (Z_TYPE_PP(zvalue) != IS_LONG) {
		LEAVE_ERROR("Weeks must be integers");	
	}
	
	switch(Z_LVAL_PP(zvalue)){
	case 1:
		return TAgnRpt::EFirst;
	case 2:
		return TAgnRpt::ESecond;
	case 3:
		return TAgnRpt::EThird;
	case 4:
		return TAgnRpt::EFourth;
	case -1:
		return TAgnRpt::ELast;
	default:
		break;
	}
	LEAVE_ERROR("Weeks must be integers -1, 1, 2, 3 or 4");
	return 0;
}




/****************************************************************************
 ****************************************************************************/

 
 
 



class CalendarDbHandle : public Refcounted {
	CalendarDbHandle() : agendaServer(0), agendaModel(0) {}
	
protected:
	~CalendarDbHandle() {
		delete agendaModel;
		agendaModel = 0;
		if (agendaServer) {
			if (agendaServer->FileLoaded()) {
				agendaServer->CloseAgenda();
			}
			agendaServer->CloseAgenda();
			agendaServer->Close();
			delete agendaServer;
			agendaServer = 0;
		}
	}
	
public:
	RAgendaServ* agendaServer;  
	CAgnModel* agendaModel;
	
	static CalendarDbHandle* NewL(
		char* filename, int filename_len,
		char* flag_str, int flag_str_len)
	{
		char flag = ParseFlagL(flag_str, flag_str_len);
		
		TBuf<KMaxFileName> filenameDes;
	  
		// Check the parameters and create new agenda file if needed.
		if (!filename) {
			filenameDes = KDefaultAgendaFile;
			if (!doesFileExistL(filenameDes)) {
				// Default file does not exist. Create it.       
				createNewAgendaFileL(filenameDes);
			}
			//XXX check that file now exists?
		} else {
			TInt error = convert_c_path_to_unicode_realpath(filenameDes, filename);
			User::LeaveIfError(error);
			switch (flag) {
			case 'e':
				// Open given db file, raise exception if the file does not exist.
				if (!doesFileExistL(filenameDes)) {
					User::Leave(KErrNotFound);
				}
			break;
			case 'c':
				// Open, create if file doesn't exist.
				if (!doesFileExistL(filenameDes)) {
					createNewAgendaFileL(filenameDes);
				}
			break;
			case 'n':
				// Create a new empty file
				createNewAgendaFileL(filenameDes);
			break;	
			}
		}
		
		// Open the db file.
		RAgendaServ* agendaServer = RAgendaServ::NewL();  
		CleanupStack::PushL(agendaServer);
		agendaServer->Connect();
		CleanupClosePushL(*agendaServer); 
		
		CAgnModel* agendaModel = CAgnModel::NewL(NULL);
		CleanupStack::PushL(agendaModel);
		agendaModel->SetServer(agendaServer);
		agendaModel->SetMode(CAgnEntryModel::EClient); 
		
		agendaModel->OpenL(filenameDes, 
		                   KDefaultTimeForEvents,
		                   KDefaultTimeForAnnivs, 
		                   KDefaultTimeForDayNote);
		
		agendaServer->WaitUntilLoaded();
		
		CalendarDbHandle* dbHandle = new (ELeave) CalendarDbHandle;
		
		CleanupStack::Pop(agendaModel);
		CleanupStack::Pop();
		CleanupStack::Pop(agendaServer);
		
		dbHandle->agendaServer = agendaServer;	
		dbHandle->agendaModel = agendaModel;
		return dbHandle;
	}

	
	void GetEntriesL(zval* return_value, zval* zUniqueId);
	void GetTodoListsL(zval* return_value);
	void FindInstancesMonthL(zval* return_value, double month, long filterInt);
	void FindInstancesDayL(zval* return_value, double day, long filterInt);
	void FindInstancesRangeL(zval* return_value, double start, double end, char* search, int search_len, long filterInt);
};






/****************************************************************************
 ****************************************************************************/

 
 
 

class CalendarEntry : public Refcounted {
	CalendarDbHandle* dbHandle;
	CAgnEntry* entryItem;
	CParaFormatLayer* paraFormatLayer;
	CCharFormatLayer* charFormatLayer;
	
	TAgnUniqueId uniqueId;
#ifdef HAVE_ALARM
	CAgnAlarm* alarm;
#endif

protected:
	~CalendarEntry() {
		delete this->entryItem;
		delete this->paraFormatLayer;
		delete this->charFormatLayer;
		#ifdef HAVE_ALARM
		delete this->alarm;
		#endif
		SAFE_CLEAR_REFERENCE(this->dbHandle);
	}

public:
	CalendarEntry() : dbHandle(0), entryItem(0), paraFormatLayer(0), charFormatLayer(0)
#ifdef HAVE_ALARM
		,alarm(0)
#endif
	{
	}

	void ConstructNewL(CalendarDbHandle* dbHandle, long type) {
		CHECK_AGENDA_STATE_DB_L(dbHandle)
		
		CAgnEntry::TType entryType = static_cast<CAgnEntry::TType>(type);
		
		// New entry must be created
		CParaFormatLayer* paraFormatLayer = CParaFormatLayer::NewL();
		CleanupStack::PushL(paraFormatLayer);
		CCharFormatLayer* charFormatLayer = CCharFormatLayer::NewL();
		CleanupStack::PushL(charFormatLayer);
		
		CAgnEntry* entry = NULL;
		switch(entryType) {
		case CAgnEntry::EAppt:       
			entry = CAgnAppt::NewL(paraFormatLayer, charFormatLayer);
			break;
		case CAgnEntry::ETodo:        
			entry = CAgnTodo::NewL(paraFormatLayer, charFormatLayer);
			break;
		case CAgnEntry::EEvent:        
			entry = CAgnEvent::NewL(paraFormatLayer, charFormatLayer);
			break;
		case CAgnEntry::EAnniv:
			entry = CAgnAnniv::NewL(paraFormatLayer, charFormatLayer);
			break;
		default:
			LEAVE_ERROR("Illegal entrytype parameter");
			break;
		}
		CleanupStack::Pop(charFormatLayer);
		CleanupStack::Pop(paraFormatLayer);
		
		this->dbHandle = dbHandle;
		dbHandle->incRef();
		this->entryItem = entry;
		this->paraFormatLayer = paraFormatLayer;
		this->charFormatLayer = charFormatLayer;
		this->uniqueId.SetNullId();
		#ifdef HAVE_ALARM
		this->alarm = NULL;
		#endif
	}
	
	// common interface
	void ConstructExistingL(CalendarDbHandle* dbHandle, zval* zUniqueId) {
		convert_to_long(zUniqueId);
		ConstructExistingL(dbHandle, Z_LVAL_P(zUniqueId));
	}
	
	// EKA1-only interface
	void ConstructExistingL(CalendarDbHandle* dbHandle, long uniqueId) {
		TAgnUniqueId uniqueIdObj(uniqueId);
		if (uniqueIdObj.IsNullId()) {
			//LEAVE_ERROR("Illegal entry id");
			User::Leave(KErrNotFound);
		}
		CHECK_AGENDA_STATE_DB_L(dbHandle)
		
		CAgnEntry* entry = dbHandle->agendaModel->FetchEntryL(uniqueIdObj);
		
		if (entry->ReplicationData().HasBeenDeleted()) {
			// This entry has been marked as deleted, but has not been removed from the database
			// since it has positive synchronization count. 
			// Anyway, we wont give access to these entrys to users since these entrys are not needed and
			// accessing their data causes crashes in some cases.
			delete entry;
			//LEAVE_ERROR("Cannot access deleted entry");
			User::Leave(KErrNotFound);
		}
		
		this->dbHandle = dbHandle;
		dbHandle->incRef();
		this->entryItem = entry;
		this->paraFormatLayer = NULL;
		this->charFormatLayer = NULL;
		this->uniqueId = uniqueIdObj;
		#ifdef HAVE_ALARM
		this->alarm = NULL;
		#endif
  	}
	
	void RemoveL() {
		CHECK_AGENDA_STATE_DB_L(this->dbHandle)
		this->dbHandle->agendaModel->DeleteEntryL(this->entryItem);//dbHandle->agendaServer->GetEntryId(this->uniqueId));
	}
	
	void CommitL() {
		CHECK_AGENDA_STATE_DB_L(this->dbHandle)
		
		if (!this->uniqueId.IsNullId()) {
			// Old entry (already exists in the database).
			this->dbHandle->agendaModel->UpdateEntryL(this->entryItem);
			return;
		}
		
		// New entry (does not exist in the database yet).		
		switch (this->entryItem->Type()) {
		case CAgnEntry::ETodo:
		{
			CAgnTodo* todo = this->entryItem->CastToTodo();
			if (todo->TodoListId().IsNullId()) {
				// Get the "default" list id (e.g. the first in the list).
				CAgnTodoListNames* todoListNames = CAgnTodoListNames::NewL();
				CleanupStack::PushL(todoListNames);
				this->dbHandle->agendaModel->PopulateTodoListNamesL(todoListNames);
				todo->SetTodoListId(todoListNames->TodoListId(0));
				CleanupStack::PopAndDestroy(); // todoListNames
				//LEAVE_ERROR("No todo-list set");
			}
			break;
		}		
		case CAgnEntry::EEvent:
		case CAgnEntry::EAnniv:
			break;
			
		case CAgnEntry::EAppt:
		{
			CAgnAppt* appt = this->entryItem->CastToAppt();
			if (appt->StartDateTime() == Time::NullTTime() || 
			    appt->EndDateTime() == Time::NullTTime()) {
				LEAVE_ERROR("Appointment must have start and end datetime");
			}
			break;
		}
		}
		this->uniqueId = 
			this->dbHandle->agendaServer->GetUniqueId(
				this->dbHandle->agendaModel->AddEntryL(this->entryItem));  
	}

	void GetIdL(zval* return_value) {
		RETURN_LONG(this->entryItem->UniqueId().Id());
	}
	
	void GetTypeL(zval* return_value) {
		RETURN_LONG(this->entryItem->Type());
	}
	
	void GetLastModifiedL(zval* return_value) {
		CHECK_AGENDA_STATE_DB_L(dbHandle)

		if (this->uniqueId.IsNullId()) {
			LEAVE_ERROR("The entry has not been committed to the database")
		}

		TAgnDateTime dateTime = this->dbHandle->agendaServer->UniqueIdLastChangedDate(this->uniqueId);
		RETURN_DOUBLE(convert_time_to_utc_real(AgnDateTime::AgnDateTimeToTTime(dateTime)));
	}
	
	void GetContentL(zval* return_value) {
		HBufC* buf = 0;
		if (this->entryItem->RichTextL()) {    
			buf = HBufC::NewLC(this->entryItem->RichTextL()->DocumentLength());
			TPtr16 bufP(buf->Des());
			this->entryItem->RichTextL()->Extract(bufP, 0);//(TDes&)buf->Des()
		}
		if (!buf) {
			RETURN_EMPTY_STRING();
		}
		TInt error = ZVAL_SYMBIAN_STRING(return_value, *buf);
		CleanupStack::PopAndDestroy(buf);
		User::LeaveIfError(error);
	}
	
	void SetContentL(char* content, int content_len) {
		HBufC* contentDes = convert_c_string_to_unicodeLC(content, content_len);
		if (!this->entryItem->RichTextL()) {
			LEAVE_ERROR("Cannot set content");
		}
		this->entryItem->RichTextL()->Reset();
		this->entryItem->RichTextL()->InsertL(0, *contentDes);
		CleanupStack::PopAndDestroy(contentDes);
	}
	
	void GetLocationL(zval* return_value) {
		TInt error = ZVAL_SYMBIAN_STRING(return_value, this->entryItem->Location());
		User::LeaveIfError(error);
	}
	
	void SetLocationL(char* location, int location_len) {
		HBufC* locDes = convert_c_string_to_unicodeLC(location, location_len);
		this->entryItem->SetLocationL(*locDes);
		CleanupStack::PopAndDestroy(locDes);
	}
	
	void GetReplicationL(zval* return_value) {
		RETURN_LONG(this->entryItem->ReplicationData().Status());
	}
	
	void SetReplicationL(long repl) {
		if (repl != TAgnReplicationData::EOpen && repl != TAgnReplicationData::EPrivate && repl != TAgnReplicationData::ERestricted) {
			LEAVE_ERROR("Illegal replication status");
		}
		
		TAgnReplicationData replicationData;
		replicationData.SetStatus(static_cast<TAgnReplicationData::TStatus>(repl));
		this->entryItem->SetReplicationData(replicationData);
	}
	
	TTime GetStartTimeL() {
		switch(this->entryItem->Type()) {
		case CAgnEntry::EAppt:
			return this->entryItem->CastToAppt()->StartDateTime();
		case CAgnEntry::EEvent:
			return this->entryItem->CastToEvent()->StartDate();
		case CAgnEntry::EAnniv:
			return this->entryItem->CastToAnniv()->StartDate();
		case CAgnEntry::ETodo:
			if (this->entryItem->CastToTodo()->IsDated()) {
				return this->entryItem->CastToTodo()->InstanceStartDate();
			} else {
				return AgnDateTime::AgnDateToTTime(AgnDateTime::NullDate());
			}
		}
		LEAVE_ERROR("Illegal entry type");
		return TTime(); // never reached, but to keep compiler silent
	}
	
	TTime GetEndTimeL() {
		switch(this->entryItem->Type()) {
		case CAgnEntry::EAppt:
			return this->entryItem->CastToAppt()->EndDateTime();
		case CAgnEntry::EEvent:
			return this->entryItem->CastToEvent()->EndDate();
		case CAgnEntry::EAnniv:
			return this->entryItem->CastToAnniv()->EndDate();
		case CAgnEntry::ETodo:
			return this->entryItem->CastToTodo()->DueDate();
		} 
		LEAVE_ERROR("Illegal entry type");
		return TTime(); // never reached, but to keep compiler silent
	}
	
	void GetStartTimeL(zval* return_value) {
		TTime startTTime = GetStartTimeL();
		if (Is_null_time(startTTime)) {
			RETURN_FALSE;
		}
		RETURN_DOUBLE(convert_time_to_utc_real(startTTime));
	}
	
	void GetEndTimeL(zval* return_value) {
		TTime endTTime = GetEndTimeL();
		if (Is_null_time(endTTime)) {
			RETURN_FALSE;
		}
		RETURN_DOUBLE(convert_time_to_utc_real(endTTime));
	}
	
	
	void SetStartAndEndTimeL(double start, double end) {
		TTime startTTime = convert_utc_real_to_time(start);
		TTime endTTime = convert_utc_real_to_time(end);
		CheckStartAndEndTimeL(startTTime, endTTime);
		
		switch (this->entryItem->Type()) {
		case CAgnEntry::EAppt:
			this->entryItem->CastToAppt()->SetStartAndEndDateTime(startTTime, endTTime);
			break;
		case CAgnEntry::EEvent:
			this->entryItem->CastToEvent()->SetStartAndEndDate(
				truncateToDate(startTTime), truncateToDate(endTTime));
			break;
		case CAgnEntry::EAnniv:
			this->entryItem->CastToAnniv()->SetStartAndEndDate(
				truncateToDate(startTTime), truncateToDate(endTTime));
			break;
		case CAgnEntry::ETodo:
			startTTime = truncateToDate(startTTime);
			endTTime = truncateToDate(endTTime);
			this->entryItem->CastToTodo()->SetDueDate(endTTime);
			this->entryItem->CastToTodo()->SetDuration(endTTime.DaysFrom(startTTime));
			break; 
		}  
	}
	
	void GetAlarmL(zval* return_value) {
		if (!this->entryItem->HasAlarm())
			RETURN_FALSE;
		RETURN_DOUBLE(convert_time_to_utc_real(this->entryItem->AlarmInstanceDateTime()));
	}

	void ClearAlarmL() {
		this->entryItem->ClearAlarm();
	}
	
	void SetAlarmL(double alarm) {
		CHECK_AGENDA_STATE_DB_L(dbHandle)
		
		TTime alarmTTime = convert_utc_real_to_time(alarm);
		TTime startTTime = GetStartTimeL();
		
		if (!Check_time_validity(startTTime)) {
			LEAVE_ERROR("Set start time for the entry before setting an alarm");
		}
		if (!Check_time_validity(alarmTTime)) {
			LEAVE_ERROR("Illegal alarm datetime value");
		}
		
		if (startTTime.DaysFrom(alarmTTime) >= TTimeIntervalDays(EARLIEST_ALARM_DAY_INTERVAL)) {
			LEAVE_ERROR("Alarm datetime too early for the entry");
		}
		
		if (alarmTTime.DateTime().Year()>startTTime.DateTime().Year() ||
		   (alarmTTime.DateTime().Year()==startTTime.DateTime().Year() &&
		    alarmTTime.DayNoInYear()>startTTime.DayNoInYear()))
		{
			LEAVE_ERROR("Alarm datetime too late for the entry");
		}
		
		this->entryItem->SetAlarm(startTTime.DaysFrom(alarmTTime), 
		    alarmTTime.DateTime().Hour()*60+alarmTTime.DateTime().Minute());
		
		
		#ifdef HAVE_ALARM
			if (!this->alarm) {
				this->alarm = CAgnAlarm::NewL(this->dbHandle->agendaModel);
			}
			this->dbHandle->agendaModel->RegisterAlarm(this->alarm);
			this->alarm->FindAndQueueNextFewAlarmsL();
			this->alarm->OrphanAlarm();
		#else
			CAgnAlarm* alarm = CAgnAlarm::NewL(this->dbHandle->agendaModel);
			CleanupStack::PushL(alarm);
			this->dbHandle->agendaModel->RegisterAlarm(alarm);
			alarm->FindAndQueueNextFewAlarmsL();
			alarm->OrphanAlarm();
			CleanupStack::PopAndDestroy(alarm);
		#endif
	}
	
	void GetCrossedOutL(zval* return_value) {
		if (this->entryItem->Type() == CAgnEntry::ETodo) {
			TTime d = this->entryItem->CastToTodo()->CrossedOutDate(); 
			if (Is_null_time(d))
				RETURN_FALSE;	
			RETURN_DOUBLE(convert_time_to_utc_real(d));
		} else {
			RETURN_BOOL(this->entryItem->IsCrossedOut());
		}
	}

	void ClearCrossedOutL() {
		if (this->entryItem->Type() == CAgnEntry::ETodo) {
			this->entryItem->CastToTodo()->UnCrossOut();  
		} else {
			this->entryItem->SetIsCrossedOut(EFalse);
		}
	}

	void SetCrossedOutL(double crossed_out) {
		if (this->entryItem->Type() == CAgnEntry::ETodo) {
			TTime theTime = convert_utc_real_to_time(crossed_out);
			if (!Check_time_validity(theTime)) {
				LEAVE_ERROR("Illegal datetime value");
			}          
			this->entryItem->CastToTodo()->CrossOut(theTime);  
		} else {
			this->entryItem->SetIsCrossedOut(ETrue);
		}  
	}
	
	void GetTodoListL(zval *return_value);
	void SetTodoListL(CalendarTodoList* todolist);
	
	void GetPriorityL(zval *return_value) {
		if (this->entryItem->Type() == CAgnEntry::ETodo) {
			RETURN_LONG(this->entryItem->CastToTodo()->Priority());
		} else {
			#if SERIES60_VERSION>12
			RETURN_LONG(this->entryItem->EventPriority());
			#else
			LEAVE_ERROR("Only todo items have priority");
			#endif
		}
	}
	
	void SetPriorityL(long priority) {
		if (this->entryItem->Type() == CAgnEntry::ETodo) {
			this->entryItem->CastToTodo()->SetPriority(priority);
		} else {
			#if SERIES60_VERSION>12
			this->entryItem->SetEventPriority(priority);
			#else
			LEAVE_ERROR("Only todo items have priority");
			#endif
		}
	}
	
	void GetRepeatL(zval* return_value);
	void ClearRepeatL();
	void SetRepeatL(TInt type, TReal startDate, TReal endDate,
				TBool eternalRepeat, TInt interval,
				CArrayFixSeg<TReal>* exceptionArray,
				CArrayFixSeg<TReal>* rdateArray, HashTable *repeatDays);
				
	void GetStatusL(zval* return_value) {
		LEAVE_ERROR("Feature not supported");
	}
	
	void SetStatusL(long status) {
		LEAVE_ERROR("Feature not supported");
	}
};





/****************************************************************************
 ****************************************************************************/

 
 
 


class CalendarInstance : public Refcounted {
	CalendarDbHandle* dbHandle;
	TAgnUniqueId uniqueId;
	TAgnDate date;
	
protected:
	~CalendarInstance() {
		SAFE_CLEAR_REFERENCE(dbHandle);
	}
public:
	CalendarInstance() : dbHandle(0) {
	}
	
	void ConstructL(CalendarDbHandle* dbHandle, TAgnUniqueId uniqueId, TAgnDate date) {
		this->dbHandle = dbHandle;
		dbHandle->incRef();
		this->uniqueId = uniqueId;
		this->date = date;
	}
	
	void GetEntryL(zval* return_value) {
		if (object_init_ex(return_value, CLASS_ENTRY(s60_calendar_Entry)) == SUCCESS) {
			GET_STRUCT(return_value, s60_calendar_Entry)->entry->ConstructExistingL(dbHandle, this->uniqueId.Id());
		} else {
			User::Leave(KErrGeneral);	
		}
	}
	
	void GetDateL(zval* return_value) {
		RETURN_DOUBLE(convert_time_to_utc_real(AgnDateTime::AgnDateToTTime(this->date)));
	}
	
	void RemoveL(long modifier) {
		//XXX may crash if uniqueId is bad
		TAgnEntryId entryId = this->dbHandle->agendaServer->GetEntryId(uniqueId);
		if (entryId.IsNullId()) {
			LEAVE_ERROR("Bad instance");
		}
		TAgnWhichInstances eWhich;
		if (modifier == REMOVE_THIS) {
			eWhich = ECurrentInstance;
		} else if (modifier == REMOVE_FUTURE) {
			eWhich = ECurrentAndFutureInstances;
		} else {
			eWhich = ECurrentAndPastInstances;
		}
		TAgnInstanceId inst(entryId, this->date);
		this->dbHandle->agendaModel->DeleteInstanceL(inst, eWhich);
	}
};





/****************************************************************************
 ****************************************************************************/

 
 
 


class CalendarTodoList : public Refcounted {
	CalendarDbHandle* dbHandle;
	
protected:
	~CalendarTodoList() {
		delete this->todoList;
		SAFE_CLEAR_REFERENCE(dbHandle);
	}
	
public:
	CAgnTodoList* todoList;
	
	CalendarTodoList() : dbHandle(0), todoList(0) {
	}
	
	void ConstructNewL(CalendarDbHandle* dbHandle)
	{
		CHECK_AGENDA_STATE_DB_L(dbHandle);
		
		CAgnTodoList* todoList = CAgnTodoList::NewL();
		CleanupStack::PushL(todoList);
		TAgnTodoListId id = dbHandle->agendaModel->AddTodoListL(todoList);
		CleanupStack::PopAndDestroy(todoList);
		
		this->todoList = dbHandle->agendaModel->FetchTodoListL(id);
		this->dbHandle = dbHandle;
		dbHandle->incRef();
	}
	
	void ConstructExistingL(CalendarDbHandle* dbHandle, long uniqueId)
	{
		TAgnUniqueId uniqueIdObj(uniqueId);
		if (uniqueIdObj.IsNullId()) {
			LEAVE_ERROR("Illegal TODO list id");
		}
		CHECK_AGENDA_STATE_DB_L(dbHandle)
		
		this->todoList = dbHandle->agendaModel->FetchTodoListL(uniqueIdObj);
		this->dbHandle = dbHandle;
		dbHandle->incRef();
	}
	
	void GetTodosL(zval* return_value);
	
	void GetIdL(zval* return_value) {
		RETURN_LONG(this->todoList->UniqueId().Id());
	}
	
	void GetNameL(zval* return_value) {
		TInt error = ZVAL_SYMBIAN_STRING(return_value, todoList->Name());
		User::LeaveIfError(error);
	}
	
	void SetNameL(char* name, int name_len) {
		CHECK_AGENDA_STATE_DB_L(this->dbHandle);
		HBufC* nameDes = convert_c_string_to_unicodeLC(name, name_len);

		this->todoList->SetName(*nameDes);
		this->dbHandle->agendaModel->UpdateTodoListL(this->todoList);
		
		CleanupStack::PopAndDestroy(nameDes);
	}
	
	void RemoveL() {
		CHECK_AGENDA_STATE_DB_L(dbHandle);
		this->dbHandle->agendaModel->DeleteTodoListL(this->todoList);
	}
};





/****************************************************************************
 ****************************************************************************/

 
 
 


static void register_constants(INIT_FUNC_ARGS) {
    // Entry types. 
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_MEETING", CAgnEntry::EAppt, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_EVENT", CAgnEntry::EEvent, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_ANNIV", CAgnEntry::EAnniv, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_TODO", CAgnEntry::ETodo, CONST_CS | CONST_PERSISTENT);

	// Filters
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_MEETING", APPTS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_EVENT", EVENTS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_ANNIV", ANNIVS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_TODO", TODOS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	
    // Replication statuses.
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REP_OPEN", TAgnReplicationData::EOpen, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REP_PRIVATE", TAgnReplicationData::EPrivate, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REP_RESTRICTED", TAgnReplicationData::ERestricted, CONST_CS | CONST_PERSISTENT);

	// Remove instance modifiers.
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REMOVE_THIS", REMOVE_THIS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REMOVE_FUTURE", REMOVE_FUTURE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REMOVE_PAST", REMOVE_PAST, CONST_CS | CONST_PERSISTENT);
}


void CalendarDbHandle::GetEntriesL(zval* return_value, zval* zUniqueId)
{
	CHECK_AGENDA_STATE_DB_L(this);
	
	if (zUniqueId == NULL) {
		TBool more = this->agendaServer->CreateEntryIterator();
		
		array_init(return_value);
		while (more) {
			TAgnEntryId entryId = this->agendaServer->EntryIteratorPosition();
			CAgnEntry* entry = this->agendaModel->FetchEntryL(entryId);
			CleanupStack::PushL(entry);
			if (!entry->ReplicationData().HasBeenDeleted()) {
				// add entry; it hasn't been deleted
				long uniqueId = this->agendaServer->GetUniqueId(entryId).Id();
				
				// add entry with uniqueId to list
				zval *new_object;
				ALLOC_INIT_ZVAL(new_object);
				// create php object
				if (object_init_ex(new_object, CLASS_ENTRY(s60_calendar_Entry)) != SUCCESS) {
					User::Leave(KErrGeneral);
				}
				TRAPD(error, GET_STRUCT(new_object, s60_calendar_Entry)->entry->ConstructExistingL(this, uniqueId));
				if (error != KErrNone) {
					zval_ptr_dtor(&new_object);			
					User::Leave(error);	
				}
				// append object to list
				add_next_index_zval(return_value, new_object);
			}
			CleanupStack::PopAndDestroy(entry);
			more = this->agendaServer->EntryIteratorNext();
		}
	} else {
		array_init(return_value);
		// add entry with uniqueId to list
		zval *new_object;
		ALLOC_INIT_ZVAL(new_object);
		// create php object
		if (object_init_ex(new_object, CLASS_ENTRY(s60_calendar_Entry)) != SUCCESS) {
			User::Leave(KErrGeneral);
		}
		TRAPD(error, GET_STRUCT(new_object, s60_calendar_Entry)->entry->ConstructExistingL(this, zUniqueId));
		if (error != KErrNone) {
			zval_ptr_dtor(&new_object);			
			if (error != KErrNotFound) {
				User::Leave(error);	
			}
		} else {
			// append object to list
			add_next_index_zval(return_value, new_object);
		}
	}
}

static void AppendInstanceL(zval* array, CalendarDbHandle* dbHandle, TAgnUniqueId uniqueId, TAgnDate date)
{
	// add instance to list
	zval *new_object;
	ALLOC_INIT_ZVAL(new_object);
	// create php object
	if (object_init_ex(new_object, CLASS_ENTRY(s60_calendar_Instance)) != SUCCESS) {
		User::Leave(KErrGeneral);
	}
	TRAPD(error, GET_STRUCT(new_object, s60_calendar_Instance)->instance->ConstructL(dbHandle, uniqueId, date));
	if (error != KErrNone) {
		zval_ptr_dtor(&new_object);			
		User::Leave(error);	
	}
	// append object to list
	add_next_index_zval(array, new_object);
}

static void FillWithInstancesL(zval* array, CalendarDbHandle* dbHandle, CAgnList<TAgnInstanceId>* ids) {
	for (TInt i = 0; i < ids->Count(); i++) {
		TAgnInstanceId instId = (*ids)[i];
		AppendInstanceL(array, dbHandle, dbHandle->agendaServer->GetUniqueId(instId), instId.Date());
	}
}


void CalendarDbHandle::FindInstancesMonthL(zval* return_value, double month, long filterInt)
{
	CHECK_AGENDA_STATE_DB_L(this);

	TTime monthTTime = convert_utc_real_to_time(month);
	if (!Check_time_validity(monthTTime)) {
		LEAVE_ERROR("Illegal date value");
	}
	
	TTime today;
	today.HomeTime();
	
	TAgnFilter filter = makeFilter(filterInt);
  
	CAgnMonthInstanceList* monthList = 
		CAgnMonthInstanceList::NewL(TTimeIntervalYears(monthTTime.DateTime().Year()), 
		                            monthTTime.DateTime().Month());
	CleanupStack::PushL(monthList);
	this->agendaModel->PopulateMonthInstanceListL(monthList, filter, today);

	array_init(return_value);
	FillWithInstancesL(return_value, this, monthList);
	CleanupStack::PopAndDestroy(monthList);
}

void CalendarDbHandle::FindInstancesDayL(zval* return_value, double day, long filterInt) 
{
	CHECK_AGENDA_STATE_DB_L(this);
	
	TTime dayTTime = convert_utc_real_to_time(day);
	if (!Check_time_validity(dayTTime)) {
		LEAVE_ERROR("Illegal date value");
	}
	
	TTime today;
	today.HomeTime();
	
	TAgnFilter filter = makeFilter(filterInt);
	
	CAgnDayList<TAgnInstanceId>* dayList =
		CAgnDayList<TAgnInstanceId>::NewL(dayTTime.DateTime());
	CleanupStack::PushL(dayList);
	this->agendaModel->PopulateDayInstanceListL(dayList, filter, today);
	
	array_init(return_value);
	FillWithInstancesL(return_value, this, dayList);
	CleanupStack::PopAndDestroy(dayList);
}


void CalendarDbHandle::FindInstancesRangeL(zval* return_value, double start, double end, char* search, int search_len, long filterInt) 
{
	CHECK_AGENDA_STATE_DB_L(this);

	HBufC* searchTextDes = convert_c_string_to_unicodeLC(search, search_len);
	TTime startTTime = convert_utc_real_to_time(start);
	TTime endTTime = convert_utc_real_to_time(end);
	startTTime = truncateToDate(startTTime);
	endTTime = truncateToDate(endTTime);

	if (!Check_time_validity(startTTime) || !Check_time_validity(endTTime)) {
		LEAVE_ERROR("Illegal date value");
	}

	if (startTTime > endTTime) {
		LEAVE_ERROR("Start date greater than end date");
	}

	TTime today;
	today.HomeTime();
	
	TAgnFilter filter = makeFilter(filterInt);

	CAgnDayList<TAgnInstanceId>* dayList =
		CAgnDayList<TAgnInstanceId>::NewL(startTTime.DateTime()); 
	CleanupStack::PushL(dayList);
	this->agendaModel->FindNextInstanceL(dayList, *searchTextDes, startTTime, endTTime, filter, today); 

	array_init(return_value);
	FillWithInstancesL(return_value, this, dayList);
	CleanupStack::PopAndDestroy(dayList);
	CleanupStack::PopAndDestroy(searchTextDes);
}

void CalendarDbHandle::GetTodoListsL(zval* return_value)
{
	CHECK_AGENDA_STATE_DB_L(this);
	
	CArrayFixFlat< TAgnTodoListId > * todoListIds = this->agendaModel->TodoListIdsL();
	CleanupStack::PushL(todoListIds);
	
	array_init(return_value);
	for (TInt i = 0; i < todoListIds->Count(); i++) { 
		TAgnTodoListId id = (*todoListIds)[i];
		long uniqueId = this->agendaServer->GetUniqueId(id).Id();
		
		/* // if some todo lists crash, use this to clean up:
		if (uniqueId == 0) {
			this->agendaModel->DeleteTodoListL(id);
			continue;
		}
		*/
		
		// add instance to list
		zval *new_object;
		ALLOC_INIT_ZVAL(new_object);
		// create php object
		if (object_init_ex(new_object, CLASS_ENTRY(s60_calendar_TodoList)) != SUCCESS) {
			User::Leave(KErrGeneral);
		}
		TRAPD(error, GET_STRUCT(new_object, s60_calendar_TodoList)->todolist->ConstructExistingL(this, uniqueId));
		if (error != KErrNone) {
			zval_ptr_dtor(&new_object);			
			User::Leave(error);	
		}
		// append object to list
		add_next_index_zval(return_value, new_object);
			
	}
	CleanupStack::PopAndDestroy(todoListIds);
}


void CalendarTodoList::GetTodosL(zval* return_value)
{
	CHECK_AGENDA_STATE_DB_L(this->dbHandle);
	
	// Get the entries in the list.
	TTime today;
	today.HomeTime();

	CAgnTodoInstanceList* todos = CAgnTodoInstanceList::NewL(this->todoList->Id());
	CleanupStack::PushL(todos);

	this->dbHandle->agendaModel->PopulateTodoInstanceListL(todos, today);

	array_init(return_value);
	FillWithInstancesL(return_value, this->dbHandle, todos);
	CleanupStack::PopAndDestroy(todos);
}

void CalendarEntry::GetTodoListL(zval *return_value) 
{
	CHECK_AGENDA_STATE_DB_L(this->dbHandle);
	if (this->entryItem->Type() != CAgnEntry::ETodo) {
		LEAVE_ERROR("Only todo entries have todo lists");
	}
	
	TAgnTodoListId id = this->entryItem->CastToTodo()->TodoListId();
	long uniqueId = this->dbHandle->agendaServer->GetUniqueId(id).Id();
	
	if (object_init_ex(return_value, CLASS_ENTRY(s60_calendar_TodoList)) != SUCCESS) {
		User::Leave(KErrGeneral);
	}
	GET_STRUCT(return_value, s60_calendar_TodoList)->todolist->ConstructExistingL(this->dbHandle, uniqueId);
}

void CalendarEntry::SetTodoListL(CalendarTodoList* todolist)
{
	CHECK_AGENDA_STATE_DB_L(this->dbHandle);
	if (this->entryItem->Type() != CAgnEntry::ETodo) {
		LEAVE_ERROR("Only todo entries have todo lists");
	}
	this->entryItem->CastToTodo()->SetTodoListId(todolist->todoList->Id());
}


void CalendarEntry::SetRepeatL(TInt type, TReal startDate, TReal endDate,
				TBool eternalRepeat, TInt interval,
				CArrayFixSeg<TReal>* exceptionArray, 
				CArrayFixSeg<TReal>* rdateArray, HashTable *repeatDays)
{
	if (type == NO_REPEAT) return;
	CAgnRptDef* rpt = CAgnRptDef::NewL();
	CleanupStack::PushL(rpt);
	
	TTime startTime = truncateToDate(convert_utc_real_to_time(startDate));
	TTime endTime = truncateToDate(convert_utc_real_to_time(endDate));
	
	if (eternalRepeat) {
		endTime = startTime;
		endTime += TTimeIntervalDays(1);
	}
	CheckStartAndEndTimeL(startTime, endTime);
  
	switch(type) {
	case DAILY_REPEAT:
	{
		TAgnDailyRpt dailyRpt;
		rpt->SetDaily(dailyRpt);
	}
	break;
	case WEEKLY_REPEAT:
	{
		TAgnWeeklyRpt weeklyRpt;
		// Add repeat days.
		if (repeatDays) {
			zval **data;
			ARRAY_FOREACH(pointer, repeatDays, data) {
				if (Z_TYPE_PP(data) != IS_LONG || !isWeekday(Z_LVAL_PP(data))) {
					LEAVE_ERROR("Bad value for weekly repeat day");
				}
				weeklyRpt.SetDay(static_cast<TDay>(Z_LVAL_PP(data)));
			}
		}
		if (weeklyRpt.NumDaysSet()==0) {
			// default repeat day.
			weeklyRpt.SetDay(startTime.DayNoInWeek());
		}       
		rpt->SetWeekly(weeklyRpt);
	}
	break;
	case MONTHLY_BY_DATES_REPEAT:
	{
		TAgnMonthlyByDatesRpt monthlyRpt;
		// Add repeat dates.
		if (repeatDays) {
			zval **data;
			ARRAY_FOREACH(pointer, repeatDays, data) {
				if (Z_TYPE_PP(data) != IS_LONG || Z_LVAL_PP(data) < 0 || Z_LVAL_PP(data) >= DAYS_IN_MONTH) {
					LEAVE_ERROR("Monthly repeat dates must be integers (0-30)");
				}
				monthlyRpt.SetDate(Z_LVAL_PP(data));
			}
		}
		if (monthlyRpt.NumDatesSet()==0) {
			// default repeat date.
			monthlyRpt.SetDate(startTime.DayNoInMonth());
		}
		rpt->SetMonthlyByDates(monthlyRpt);
	}
	break;
	case MONTHLY_BY_DAYS_REPEAT:
	{
		TAgnMonthlyByDaysRpt monthlyRpt;
		// Add repeat dates.
		if (repeatDays) {
			zval **data;
			ARRAY_FOREACH(pointer, repeatDays, data) {
				if (Z_TYPE_PP(data) != IS_ARRAY) {
					LEAVE_ERROR("Monthly repeat days be in arrays");
				}
				zval** zvalue;
				if (zend_hash_find(Z_ARRVAL_PP(data), KWeek, sizeof(KWeek), (void**)&zvalue) != SUCCESS) {
					LEAVE_ERROR("Week is missing from monthly repeat data");
				}
				long week = convertToWeekL(zvalue);

				if (zend_hash_find(Z_ARRVAL_PP(data), KDay, sizeof(KDay), (void**)&zvalue) != SUCCESS) {
					LEAVE_ERROR("Day is missing from monthly repeat data");
				}
				if (Z_TYPE_PP(zvalue) != IS_LONG || !isWeekday(Z_LVAL_PP(zvalue))) {
					LEAVE_ERROR("Days must be integers on range 0-6");	
				}
				long day = Z_LVAL_PP(zvalue);
				monthlyRpt.SetDay(static_cast<TDay>(day), 
				                  static_cast<TAgnRpt::TWeekInMonth>(week));
			}
		}
		if (monthlyRpt.NumDaysSet()==0) {
			// default repeat day.
			monthlyRpt.SetDay(startTime.DayNoInWeek(), 
			                  static_cast<TAgnRpt::TWeekInMonth>(startTime.DayNoInMonth()/DAYS_IN_WEEK));
		}         
		rpt->SetMonthlyByDays(monthlyRpt);
	}
	break;
	case YEARLY_BY_DATE_REPEAT:
	{
		TAgnYearlyByDateRpt yearlyRpt;
		rpt->SetYearlyByDate(yearlyRpt);
	}
    break;
    case YEARLY_BY_DAY_REPEAT:
	{
		TAgnYearlyByDayRpt yearlyRpt;
		
		if (repeatDays) {
			zval** zvalue;
			if (zend_hash_find(repeatDays, KWeek, sizeof(KWeek), (void**)&zvalue) != SUCCESS) {
				LEAVE_ERROR("Week is missing from yearly repeat data");
			}
			long week = convertToWeekL(zvalue);
			
			if (zend_hash_find(repeatDays, KDay, sizeof(KDay), (void**)&zvalue) != SUCCESS) {
				LEAVE_ERROR("Day is missing from yearly repeat data");
			}
			if (Z_TYPE_PP(zvalue) != IS_LONG || !isWeekday(Z_LVAL_PP(zvalue))) {
				LEAVE_ERROR("Day must be integer on range 0-6");	
			}
			long day = Z_LVAL_PP(zvalue);

			if (zend_hash_find(repeatDays, KMonth, sizeof(KMonth), (void**)&zvalue) != SUCCESS) {
				LEAVE_ERROR("Month is missing from yearly repeat data");
			}
			if (Z_TYPE_PP(zvalue) != IS_LONG || !isMonth(Z_LVAL_PP(zvalue))) {
				LEAVE_ERROR("Month must be integer on range 0-11");	
			}
			long month = Z_LVAL_PP(zvalue);
			yearlyRpt.SetStartDay(static_cast<TDay>(day), 
			                 static_cast<TAgnRpt::TWeekInMonth>(week), 
			                 static_cast<TMonth>(month), 
			                 startTime.DateTime().Year());
			                 
			// Ensure that the specified day is between start date and end date.
			if (yearlyRpt.StartDate()<startTime) {
				yearlyRpt.SetStartDay(static_cast<TDay>(day), 
				               static_cast<TAgnRpt::TWeekInMonth>(week), 
				               static_cast<TMonth>(month), 
				               startTime.DateTime().Year()+1);
				
				if (yearlyRpt.StartDate()>endTime-TTimeIntervalDays(1)) { // must be endTime-1 day or panic occurs!!!
					LEAVE_ERROR("Yearly day of repeat must be between (repeat start date) and (repeat end date - 1 day)");
				}
			}
			
			if (yearlyRpt.StartDate()>endTime-TTimeIntervalDays(1)) {
				yearlyRpt.SetStartDay(static_cast<TDay>(day), 
				               static_cast<TAgnRpt::TWeekInMonth>(week), 
				               static_cast<TMonth>(month), 
				               endTime.DateTime().Year()-1);
				
				if (yearlyRpt.StartDate()<startTime) {
					LEAVE_ERROR("Yearly day of repeat must be between (repeat start date) and (repeat end date - 1 day)");
				}
			}
		}else{
			// default repeat date.
			yearlyRpt.SetStartDay(startTime.DayNoInWeek(), 
			                     static_cast<TAgnRpt::TWeekInMonth>(startTime.DayNoInMonth()/DAYS_IN_WEEK),
			                     startTime.DateTime().Month(), 
			                     startTime.DateTime().Year());
		}
		rpt->SetYearlyByDay(yearlyRpt);
	}
	break;
	default:
		LEAVE_ERROR("Illegal repeat type");
	}
	if (type != YEARLY_BY_DAY_REPEAT) { //XXX is this really ok?
		rpt->SetStartDate(startTime);
	}
	rpt->SetEndDate(endTime);
	if (eternalRepeat){
		rpt->SetRepeatForever(ETrue);
	}
	rpt->SetInterval(interval);
  
	if (exceptionArray) {
		for (TInt i=0; i<exceptionArray->Count(); i++){
			TTime exceptionTime = convert_utc_real_to_time(exceptionArray->At(i));
			if (!Check_time_validity(exceptionTime)){
				LEAVE_ERROR("illegal exception date");
			}
			
			TAgnException exception;
			exception.SetDate(TAgnDateTime(exceptionTime).Date());
			
			rpt->AddExceptionL(exception);
		}
	}
	this->entryItem->SetRptDefL(rpt);
	CleanupStack::PopAndDestroy(rpt);
}

void CalendarEntry::GetRepeatL(zval* return_value)
{
	if (!this->entryItem->RptDef()) {
		RETURN_FALSE;
	}
	
	array_init(return_value);
	const TDesC8* typePtr;
	switch(this->entryItem->RptDef()->Type()){
	case CAgnRptDef::EDaily: typePtr = &KDaily; break;
	case CAgnRptDef::EWeekly: typePtr = &KWeekly; break;
	case CAgnRptDef::EMonthlyByDates: typePtr = &KMonthlyByDates; break;
	case CAgnRptDef::EMonthlyByDays: typePtr = &KMonthlyByDays; break;
	case CAgnRptDef::EYearlyByDate: typePtr = &KYearlyByDate; break;
	case CAgnRptDef::EYearlyByDay: typePtr = &KYearlyByDay; break;
	default: typePtr = &KUnknownRep; break;
	}
	add_assoc_string(return_value, KRepeatType, (char*)typePtr->Ptr(), 1);
	add_assoc_double(return_value, KStartDate, convert_time_to_utc_real(this->entryItem->RptDef()->StartDate()));
	if (!this->entryItem->RptDef()->RepeatForever()) {
		add_assoc_double(return_value, KEndDate, convert_time_to_utc_real(this->entryItem->RptDef()->EndDate()));
	}
	add_assoc_long(return_value, KInterval, this->entryItem->RptDef()->Interval());
		
	// Exceptions.
	const CAgnExceptionList* exceptionList = this->entryItem->RptDef()->Exceptions(); 
	if (exceptionList && exceptionList->Count() > 0) {
		zval* zex;
		ALLOC_INIT_ZVAL(zex);
		array_init(zex);
		for (TInt i=0;i<exceptionList->Count();i++) {
			const TAgnException& aException = exceptionList->At(i);      
			double date = convert_time_to_utc_real(AgnDateTime::AgnDateToTTime(aException.Date()));
			add_next_index_double(zex, date);
		}
		add_assoc_zval(return_value, KExceptions, zex);
    }

	// Recurrence.
	
	zval* zre;
	ALLOC_INIT_ZVAL(zre);
	array_init(zre);	
	TDay dayArr[DAYS_IN_WEEK] = {EMonday, ETuesday, EWednesday, EThursday, EFriday, ESaturday, ESunday};
	
	switch(this->entryItem->RptDef()->Type()){
	case CAgnRptDef::EWeekly:
	{        
		for(TInt i = 0; i < DAYS_IN_WEEK; i++) {
			if(this->entryItem->RptDef()->Weekly().IsDaySet(dayArr[i])) {
				add_next_index_long(zre, dayArr[i]);
			}
		}
	}  
	break;
	case CAgnRptDef::EMonthlyByDates:
	{        
		for(TInt i = 0; i < DAYS_IN_MONTH; i++){
			if(this->entryItem->RptDef()->MonthlyByDates().IsDateSet(i)) {
				add_next_index_long(zre, i);
			}
		}
	}
	break;
	case CAgnRptDef::EMonthlyByDays:
	{        
		for(TInt i = 0; i < WEEKS_IN_MONTH; i++){
			for(TInt j = 0; j < DAYS_IN_WEEK; j++){
				if (this->entryItem->RptDef()->MonthlyByDays().IsDaySet(
						dayArr[j], static_cast<TAgnRpt::TWeekInMonth>(i)))
				{
					zval* zsub;
					ALLOC_INIT_ZVAL(zsub);
					array_init(zsub);	
					add_assoc_long(zsub, KDay, dayArr[j]);
					add_assoc_long(zsub, KWeek, i);
					add_next_index_zval(zre, zsub);
				}
			}
		} 
	}
	break;
    case CAgnRptDef::EYearlyByDay:
	{
		TDay day;
		TAgnRpt::TWeekInMonth week;
		TMonth month;
		TInt year;
		this->entryItem->RptDef()->YearlyByDay().GetStartDay(day, week, month, year);
		add_assoc_long(zre, KDay, day);
		add_assoc_long(zre, KWeek, week);
		add_assoc_long(zre, KMonth, month);
	}
	break;
    case CAgnRptDef::EDaily:
    case CAgnRptDef::EYearlyByDate:
    break;
	}
	
	if (zend_hash_num_elements(Z_ARRVAL_P(zre)) == 0) {
		zval_ptr_dtor(&zre);		
	} else {
		add_assoc_zval(return_value, KRepeatDays, zre);
	}
}

void CalendarEntry::ClearRepeatL()
{
	this->entryItem->ClearRepeat();
}
