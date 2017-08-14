/**
 * ====================================================================
 *  PHP API to calendar handling, modified from the corresponding
 *  Python API. Allows reading and modifying of calendar and todo
 *  information.
 * --------------------------------------------------------------------
 *
 * Copyright (c) 2006 Nokia Corporation
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
 * ====================================================================
 */

/**
//EXAMPLE

<pre><?php

function format_time($t) {
	if (!$t) {
		return "-";
	}
	return strftime ("%a, %d %b %Y at %H:%M:%S", $t);
}

function print_entry($e) {
	echo "ID: ".$e->id()."\n";
	echo "TYPE: ".$e->type()."\n";
	echo "MODIFIED: ".format_time($e->last_modified())."\n";
	echo "CONTENT: ".$e->content()."\n";
	echo "LOCATION: ".$e->location()."\n";
	echo "START TIME: ".format_time($e->start_time())."\n";
	echo "END TIME: ".format_time($e->end_time())."\n";
	echo "ALARM TIME: ".format_time($e->alarm())."\n";
	echo "REPLICATION: ".$e->replication()."\n";
	echo "PRIORITY: ".$e->priority()."\n";	
	if ($e->type() == S60_CALENDAR_TYPE_TODO) {
		echo "TODO LIST: ".$e->todo_list()->name()."\n";	
	}
	echo "REPEAT: "; var_dump($e->repeat());
}

// Open calendar database
$db = s60_calendar_open("ctest", "n");

define('HOUR', 3600);
define('DAY', HOUR*24);
define('WEEK', DAY*7);
define('NOW', time());

// Add some entries


// Add a meeting that repeats every two weeks
// for 5 weeks, starting today. Start time is
// one hour from now, meetings lasts an hour.

$c = $db->add_entry(S60_CALENDAR_TYPE_MEETING);
$c->set_content("my meeting");
$c->set_location("my meeting location");
$c->set_start_end_time(NOW+HOUR, NOW+2*HOUR);
$c->set_priority(100);
$c->set_replication(S60_CALENDAR_REP_OPEN);
$c->set_alarm(NOW+HOUR/2);
$c->set_repeat(array(
		"type"=>"weekly",
		"start"=>NOW,
		"end"=>NOW + WEEK*5,
		"interval"=>2));
$c->commit();


// Add an anniversary for the day after tomorrow.

$c = $db->add_entry(S60_CALENDAR_TYPE_ANNIV);
$c->set_content("my anniversary");
$c->set_location("my anniversary location");
$c->set_start_end_time(NOW+DAY*2, NOW+DAY*2);
$c->set_replication(S60_CALENDAR_REP_RESTRICTED);
$c->set_priority(50);
$c->commit();


// Print all calendar entries.

echo "\n\nALL ENTRIES:\n";
echo "================================\n";
foreach ($db->entries() as $e) {
	print_entry($e);
	echo "--------------------------------\n";
}


?></pre>

*/

/**
//CONSTANTS

    // Entry types (REMINDER type only in S60 version 3.x)
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_MEETING", CCalEntry::EAppt, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_EVENT", CCalEntry::EEvent, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_ANNIV", CCalEntry::EAnniv, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_TODO", CCalEntry::ETodo, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_TYPE_REMINDER", CCalEntry::EReminder, CONST_CS | CONST_PERSISTENT);

	// Filters (REMINDER only in S60 version 3.x. TODO only finds TODOs whose status is not TODO_COMPLETED)
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_MEETING", APPTS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_EVENT", EVENTS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_ANNIV", ANNIVS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_TODO", TODOS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_FILTER_REMINDER", REMINDERS_INC_FILTER, CONST_CS | CONST_PERSISTENT);
	
    // Replication statuses
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REP_OPEN", CCalEntry::EOpen, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REP_PRIVATE", CCalEntry::EPrivate, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REP_RESTRICTED", CCalEntry::ERestricted, CONST_CS | CONST_PERSISTENT);
	
	// Remove instance modifiers.
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REMOVE_THIS", REMOVE_THIS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REMOVE_FUTURE", REMOVE_FUTURE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_REMOVE_PAST", REMOVE_PAST, CONST_CS | CONST_PERSISTENT);
	
    // Statuses (only in S60 version 3.x)
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_CANCELLED", CCalEntry::ECancelled, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_TODO_NEED_ACTION", CCalEntry::ETodoNeedsAction, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_TODO_COMPLETED", CCalEntry::ETodoCompleted, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_TODO_IN_PROGRESS", CCalEntry::ETodoInProcess, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_TENTATIVE", CCalEntry::ETentative, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_CONFIRMED", CCalEntry::EConfirmed, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CALENDAR_STATUS_NULL", CCalEntry::ENullStatus, CONST_CS | CONST_PERSISTENT);


*/

#include <e32base.h>
#include "s60ext_tools.h"


// Keys of repeat dictionary
#define KRepeatType "type"
#define KStartDate "start"
#define KEndDate "end"
#define KRepeatDays "days"
#define KExceptions "exceptions"
#define KRDates "rdates"
#define KInterval "interval"

#define KWeek "week"
#define KMonth "month"
#define KDay "day"

// Repeat types
_LIT8(KNoRepeat, "no_repeat");
_LIT8(KDaily, "daily");
_LIT8(KWeekly, "weekly");
//_LIT8(KMonthly, "monthly");
//_LIT8(KYearly, "yearly");
_LIT8(KMonthlyByDates, "monthly_by_dates");
_LIT8(KMonthlyByDays, "monthly_by_days");
_LIT8(KYearlyByDate, "yearly_by_date");
_LIT8(KYearlyByDay, "yearly_by_day");
_LIT8(KUnknownRep, "unknown");

#define REMOVE_THIS 0
#define REMOVE_FUTURE 1
#define REMOVE_PAST 2

// Repeat types as integers
#define NO_REPEAT 0
#define DAILY_REPEAT 1
#define WEEKLY_REPEAT 2
#define MONTHLY_BY_DATES_REPEAT 3
#define MONTHLY_BY_DAYS_REPEAT 4
#define YEARLY_BY_DATE_REPEAT 5
#define YEARLY_BY_DAY_REPEAT 6

#define DAYS_IN_WEEK 7
#define DAYS_IN_MONTH 31
#define WEEKS_IN_MONTH 5
#define MONTHS_IN_YEAR 12



/*
 * Checks if the value represents a weekday.
 */
static TBool isWeekday(TInt value)
{
	switch (value) {
	case EMonday:
	case ETuesday:
	case EWednesday:
	case EThursday:
	case EFriday:
	case ESaturday:
	case ESunday:
		return ETrue;
	}
	return EFalse;
}




/*
 * Checks if the value represents a month.
 */
static TBool isMonth(TInt value)
{
	switch(value){
	case EJanuary:
	case EFebruary:
	case EMarch:
	case EApril:
	case EMay:
	case EJune:
	case EJuly:
	case EAugust:
	case ESeptember:
	case EOctober:
	case ENovember:
	case EDecember:
		return ETrue;
	}
	return EFalse;
}
	
extern "C" {

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h" 
#include "calendar.h"

BEGIN_MODULE_FUNCTIONS(s60_calendar)
	PHP_FE(s60_calendar_open, NULL)
END_MODULE_FUNCTIONS()


DEFINE_MODULE(s60_calendar, "1.0", PHP_MINIT(s60_calendar), NULL, NULL, NULL)


static char ParseFlagL(char* flag_str, int flag_str_len)
{
	if (flag_str && flag_str_len > 1) {
		LEAVE_ERROR("Use only one flag");
	} else if (flag_str && flag_str_len == 1) {
		if (flag_str[0] != 'n' && flag_str[0] != 'c' && flag_str[0] != 'e') {
			LEAVE_ERROR("Illegal flag (use 'n', 'c', 'e')");
		}
		return flag_str[0];
	}
	return 'e';	
}

class CalendarDbHandle;
CLASS_REGISTER_FUNCTION(s60_calendar_Database);
BEGIN_CLASS_STRUCT(s60_calendar_Database)
	CalendarDbHandle* dbHandle; //if 0, db is closed
END_CLASS_STRUCT()
DEFINE_CLASS_DATA(s60_calendar_Database)

class CalendarEntry;
CLASS_REGISTER_FUNCTION(s60_calendar_Entry);
BEGIN_CLASS_STRUCT(s60_calendar_Entry)
	CalendarEntry* entry;
END_CLASS_STRUCT()
DEFINE_CLASS_DATA(s60_calendar_Entry)

class CalendarTodoList;
CLASS_REGISTER_FUNCTION(s60_calendar_TodoList);
BEGIN_CLASS_STRUCT(s60_calendar_TodoList)
	CalendarTodoList* todolist;
END_CLASS_STRUCT()
DEFINE_CLASS_DATA(s60_calendar_TodoList)

class CalendarInstance;
CLASS_REGISTER_FUNCTION(s60_calendar_Instance);
BEGIN_CLASS_STRUCT(s60_calendar_Instance)
	CalendarInstance* instance;
END_CLASS_STRUCT()
DEFINE_CLASS_DATA(s60_calendar_Instance)


}

//c++ linkage:
#ifdef EKA2
	#include "calendar-eka2.h"
#else
	#include "calendar-eka1.h"
#endif /* EKA2 */

extern "C" {

PHP_MINIT_FUNCTION(s60_calendar)
{
	register_constants(INIT_FUNC_ARGS_PASSTHRU);
	CLASS_REGISTER_CALL(s60_calendar_Database);
	CLASS_REGISTER_CALL(s60_calendar_Entry);
	CLASS_REGISTER_CALL(s60_calendar_TodoList);
	CLASS_REGISTER_CALL(s60_calendar_Instance);
	return SUCCESS;
}


/**********************************************************************
 * Class s60_calendar_Database
 * This class is a connection to Series 60 calendar database.
 **********************************************************************/


BEGIN_CLASS_FUNCTIONS(s60_calendar_Database)
	PHP_ME(s60_calendar_Database, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, open, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, get_entry, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, add_entry, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, entries, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, find_instances_month, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, find_instances_day, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, find_instances_range, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, get_todo_list, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, add_todo_list, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Database, todo_lists, NULL, ZEND_ACC_PUBLIC)
END_CLASS_FUNCTIONS()

DEFINE_OBJECT_FREE_FUNCTION(s60_calendar_Database, {
	SAFE_CLEAR_REFERENCE(self->dbHandle);
})
DEFINE_OBJECT_NEW_FUNCTION(s60_calendar_Database, {
	self->dbHandle = 0;
})
DEFINE_CLASS_REGISTER_FUNCTION(s60_calendar_Database)

/**
 * s60_calendar_Database::__construct()
 * Constructs a closed calendar database object.
 */
PHP_METHOD(s60_calendar_Database, __construct)
{
	GET_METHOD_ARGS_0(s60_calendar_Database);
}

// Codewarrior crashes in internal error without this wrapper
static CalendarDbHandle* CalendarDbHandleNewL(
		char* filename, int filename_len,
		char* flag_str, int flag_str_len)
{
	return CalendarDbHandle::NewL(filename, filename_len, flag_str, flag_str_len);	
}


/**
 * <bool> s60_calendar_Database::open([<string> filename [,<string> flag='e']])
 * open() - opens the default calendar database file
 * open($filename [, 'e']) - opens file if it exists. Returns false if it doesn't.
 * open($filename , 'c') - opens file, creates if the file does not exist.
 * open($filename , 'n') - creates new empty database file.
 */
PHP_METHOD(s60_calendar_Database, open)
{
	char* filename = NULL;
	int filename_len;
	char* flag_str = NULL;
	int flag_str_len;
	
	GET_METHOD_ARGS_4(s60_calendar_Database, "O|ss", &filename, &filename_len, &flag_str, &flag_str_len);
	TRAPD(error, {
		SAFE_CLEAR_REFERENCE(self->dbHandle);
		self->dbHandle = CalendarDbHandleNewL(filename, filename_len, flag_str, flag_str_len);
	});
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}

/**
 * <s60_calendar_Database> s60_calendar_open([<string> filename [,<string> flag='e']])
 * Returns an open <s60_calendar_Database> object. (or NULL on error)
 * s60_calendar_open() - opens the default calendar database file
 * s60_calendar_open($filename [, 'e']) - opens file if it exists. Returns false if it doesn't.
 * s60_calendar_open($filename , 'c') - opens file, creates if the file does not exist.
 * s60_calendar_open($filename , 'n') - creates new empty database file.
 */
PHP_FUNCTION(s60_calendar_open)
{
	char* filename = NULL;
	int filename_len;
	char* flag_str = NULL;
	int flag_str_len;
	
	GET_FUNCTION_ARGS_4("|ss", &filename, &filename_len, &flag_str, &flag_str_len);
	zval new_object;
	INIT_ZVAL(new_object);
	if (object_init_ex(&new_object, CLASS_ENTRY(s60_calendar_Database)) == SUCCESS) {
		TRAPD(error, {
			GET_STRUCT(&new_object, s60_calendar_Database)->dbHandle =
				CalendarDbHandleNewL(filename, filename_len, flag_str, flag_str_len);
		});
		if (error != KErrNone) {
			zval_dtor(&new_object);	
			RETURN_SYMBIAN_ERROR(error);
		}
		*return_value = new_object;
	} else {
		RETURN_ERROR("Could not allocate Database");
	}
}


/**
 * <bool> s60_calendar_Database::close()
 */
PHP_METHOD(s60_calendar_Database, close)
{
	GET_METHOD_ARGS_0(s60_calendar_Database);
	SAFE_CLEAR_REFERENCE(self->dbHandle);
	RETURN_TRUE;
}


#define ASSERT_DB_OPEN { if (!self->dbHandle) { RETURN_ERROR("Database not open"); } }

/**
 * <s60_calendar_Entry> s60_calendar_Database::get_entry(<id> id)
 * Returns the first Entry object corresponding to given ID.
 * On S60 version 2.x this is also the only such Entry object.
 * On S60 version 3.x, it's the parent entry of the group. Use entries() to fetch
 * the other entries.
 * ID is of whatever type that <s60_calendar_Entry>::id() returns (differs between
 * S60 versions 2.x and 3.x).
 */
PHP_METHOD(s60_calendar_Database, get_entry)
{
	zval* uniqueId;
	GET_METHOD_ARGS_1(s60_calendar_Database, "Oz", &uniqueId);
	ASSERT_DB_OPEN;
	
	TInt error = KErrGeneral;
	if (object_init_ex(return_value, CLASS_ENTRY(s60_calendar_Entry)) == SUCCESS) {
		TRAP(error, GET_STRUCT(return_value, s60_calendar_Entry)->entry->ConstructExistingL(self->dbHandle, uniqueId));
	}
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <s60_calendar_Entry> s60_calendar_Database::add_entry(<int> entryType)
 * Creates new entry (but does not commit it into the database).
 * entryType is e.g. S60_CALENDAR_TYPE_MEETING
 */
PHP_METHOD(s60_calendar_Database, add_entry)
{
	long entryType;
	GET_METHOD_ARGS_1(s60_calendar_Database, "Ol", &entryType);
	ASSERT_DB_OPEN;
	
	TInt error = KErrGeneral;
	if (object_init_ex(return_value, CLASS_ENTRY(s60_calendar_Entry)) == SUCCESS) {
		TRAP(error, GET_STRUCT(return_value, s60_calendar_Entry)->entry->ConstructNewL(self->dbHandle, entryType));
	}
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <array of s60_calendar_Entry> s60_calendar_Database::entries([<id> id])
 * Returns all the entries corresponding to the specified ID.
 * If the ID isn't specified, this returns all the entries in the database.
 *
 * On S60 3.x there may be several entries corresponding
 *  to the same ID, and this will return them all.
 *
 * ID is of whatever type <s60_calendar_Entry>::id() returns (differs between S60 2.x and 3.x).
 */
PHP_METHOD(s60_calendar_Database, entries)
{
	zval* uniqueId = NULL;
	GET_METHOD_ARGS_1(s60_calendar_Database, "O|z", &uniqueId);
	ASSERT_DB_OPEN;
	
	TRAPD(error, self->dbHandle->GetEntriesL(return_value, uniqueId));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <array of s60_calendar_Instance> s60_calendar_Database::find_instances_month(<float> month [, <int> flag])
 * Returns instances of the given month, as filtered by "flag". "flag" can be e.g. 
 * (S60_CALENDAR_FILTER_MEETING | S60_CALENDAR_FILTER_EVENT). If flag is not set,
 * all instances are included.
 */
PHP_METHOD(s60_calendar_Database, find_instances_month)
{
	long flag = 0;
	double month;
	GET_METHOD_ARGS_2(s60_calendar_Database, "Od|l", &month, &flag);
	ASSERT_DB_OPEN;
	
	TRAPD(error, self->dbHandle->FindInstancesMonthL(return_value, month, flag));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <array of s60_calendar_Instance> s60_calendar_Database::find_instances_day(<float> day [, <int> flag])
 * Returns instances of the given day, as filtered by "flag". "flag" can be e.g. 
 * (S60_CALENDAR_FILTER_MEETING | S60_CALENDAR_FILTER_EVENT). If flag is not set,
 * all instances are included.
 */
PHP_METHOD(s60_calendar_Database, find_instances_day)
{
	long flag = 0;
	double month;
	GET_METHOD_ARGS_2(s60_calendar_Database, "Od|l", &month, &flag);
	ASSERT_DB_OPEN;
	
	TRAPD(error, self->dbHandle->FindInstancesDayL(return_value, month, flag));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <array of s60_calendar_Instance> s60_calendar_Database::find_instances_range(<float> start, <float> end, <string> search [, <int> flag])
 * Returns instances of the given range, as filtered by "flag" and "search" string. "flag" can be e.g. 
 * (S60_CALENDAR_FILTER_MEETING | S60_CALENDAR_FILTER_EVENT). If flag is not set,
 * all instances are included.
 */
PHP_METHOD(s60_calendar_Database, find_instances_range)
{
	long flag = 0;
	double start, end;
	char *search;
	int search_len;
	GET_METHOD_ARGS_5(s60_calendar_Database, "Odds|l", &start, &end, &search, &search_len, &flag);
	ASSERT_DB_OPEN;
	
	TRAPD(error, self->dbHandle->FindInstancesRangeL(return_value, start, end, search, search_len, flag));
	RETURN_IF_SYMBIAN_ERROR(error);
}


/**
 * <s60_calendar_TodoList> s60_calendar_Database::get_todo_list(<int> id)
 * Returns a TodoList object corresponding to given ID.
 * Only supported in S60 version 2.x.
 */
PHP_METHOD(s60_calendar_Database, get_todo_list)
{
	long uniqueId;
	GET_METHOD_ARGS_1(s60_calendar_Database, "Ol", &uniqueId);
	ASSERT_DB_OPEN;
	
	TInt error = KErrGeneral;
	if (object_init_ex(return_value, CLASS_ENTRY(s60_calendar_TodoList)) == SUCCESS) {
		TRAP(error, GET_STRUCT(return_value, s60_calendar_TodoList)->todolist->ConstructExistingL(self->dbHandle, uniqueId));
	}
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <s60_calendar_TodoList> s60_calendar_Database::add_todo_list()
 * Creates a new todo list and adds it to the database.
 * Only supported in S60 version 2.x.
 */
PHP_METHOD(s60_calendar_Database, add_todo_list)
{
	GET_METHOD_ARGS_0(s60_calendar_Database);
	ASSERT_DB_OPEN;
	
	TInt error = KErrGeneral;
	if (object_init_ex(return_value, CLASS_ENTRY(s60_calendar_TodoList)) == SUCCESS) {
		TRAP(error, GET_STRUCT(return_value, s60_calendar_TodoList)->todolist->ConstructNewL(self->dbHandle));
	}
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <array of s60_calendar_TodoList> s60_calendar_Database::todo_lists()
 * Returns all todo lists.
 * Only supported in S60 version 2.x.
 */
PHP_METHOD(s60_calendar_Database, todo_lists)
{
	GET_METHOD_ARGS_0(s60_calendar_Database);
	ASSERT_DB_OPEN;
	
	TRAPD(error, self->dbHandle->GetTodoListsL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}


/**********************************************************************
 * Class s60_calendar_Entry
 * This class represents a calendar entry. An entry may
 * repeat on several days, and each repeated instance is represented
 * by the class <s60_calendar_Instance>. Entry contains the information
 * that is common for all instances (that is, all information except
 * the instance's date).
 **********************************************************************/

BEGIN_CLASS_FUNCTIONS(s60_calendar_Entry)
	PHP_ME(s60_calendar_Entry, __construct, NULL, ZEND_ACC_PRIVATE)
	PHP_ME(s60_calendar_Entry, remove, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, commit, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, id, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, type, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, last_modified, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, content, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, set_content, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, location, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, set_location, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, replication, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, set_replication, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, start_time, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, end_time, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, set_start_end_time, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, alarm, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, set_alarm, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, crossed_out, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, set_crossed_out, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, todo_list, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, set_todo_list, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, priority, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, set_priority, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, repeat, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, set_repeat, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, status, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Entry, set_status, NULL, ZEND_ACC_PUBLIC)
END_CLASS_FUNCTIONS()

DEFINE_OBJECT_FREE_FUNCTION(s60_calendar_Entry, {
	SAFE_CLEAR_REFERENCE(self->entry);
})
DEFINE_OBJECT_NEW_FUNCTION(s60_calendar_Entry, {
	self->entry = new CalendarEntry;
})
DEFINE_CLASS_REGISTER_FUNCTION(s60_calendar_Entry)

PHP_METHOD(s60_calendar_Entry, __construct) { }

#define ASSERT_ENTRY_OPEN { if (!self->entry) { RETURN_ERROR("Entry not open"); } }


/**
 * s60_calendar_Entry::remove()
 * Removes the entry (all its instances).
 * The instances of this entry should not be modified anymore once
 * the entry is gone.
 */
PHP_METHOD(s60_calendar_Entry, remove)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->RemoveL());
	RETURN_IF_SYMBIAN_ERROR(error);
	SAFE_CLEAR_REFERENCE(self->entry);
	RETURN_TRUE;
}

/**
 * s60_calendar_Entry::commit()
 * Commits changes to the entry.
 */
PHP_METHOD(s60_calendar_Entry, commit)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->CommitL());
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}

/**
 * <id> s60_calendar_Entry::id()
 * Returns entry's unique ID, which may be string or int. To be used with
 * <s60_calendar_Database>::get_entry()
 */
PHP_METHOD(s60_calendar_Entry, id)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetIdL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <int> s60_calendar_Entry::type()
 * Returns entry's type, e.g. S60_CALENDAR_TYPE_MEETING
 */
PHP_METHOD(s60_calendar_Entry, type)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetTypeL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <float> s60_calendar_Entry::last_modified()
 */
PHP_METHOD(s60_calendar_Entry, last_modified)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetLastModifiedL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <string> s60_calendar_Entry::content()
 */
PHP_METHOD(s60_calendar_Entry, content)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetContentL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * s60_calendar_Entry::set_content(<string> content)
 */
PHP_METHOD(s60_calendar_Entry, set_content)
{
	char* content;
	int content_len;
	GET_METHOD_ARGS_2(s60_calendar_Entry, "Os", &content, &content_len);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->SetContentL(content, content_len));
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}
/**
 * <string> s60_calendar_Entry::location()
 */
PHP_METHOD(s60_calendar_Entry, location)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetLocationL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * s60_calendar_Entry::set_location(<string> loc)
 */
PHP_METHOD(s60_calendar_Entry, set_location)
{
	char* location;
	int location_len;
	GET_METHOD_ARGS_2(s60_calendar_Entry, "Os", &location, &location_len);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->SetLocationL(location, location_len));
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}

/**
 * <int> s60_calendar_Entry::replication()
 * Returns replication status, e.g. S60_CALENDAR_REP_OPEN
 */
PHP_METHOD(s60_calendar_Entry, replication)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetReplicationL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}


/**
 * s60_calendar_Entry::set_replication(<int> repl)
 * Sets replication status, e.g. S60_CALENDAR_REP_OPEN
 */
PHP_METHOD(s60_calendar_Entry, set_replication)
{
	long repl;
	GET_METHOD_ARGS_1(s60_calendar_Entry, "Ol", &repl);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->SetReplicationL(repl));
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}

/**
 * <float> s60_calendar_Entry::start_time()
 */
PHP_METHOD(s60_calendar_Entry, start_time)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetStartTimeL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <float> s60_calendar_Entry::end_time()
 */
PHP_METHOD(s60_calendar_Entry, end_time)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetEndTimeL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <string> s60_calendar_Entry::set_start_end_time(<float> start, <float> end)
 * Sets both start and end time for the entry. For a repeating entry,
 * start and end time should be on the same day.
 */
PHP_METHOD(s60_calendar_Entry, set_start_end_time)
{
	double start, end;
	GET_METHOD_ARGS_2(s60_calendar_Entry, "Odd", &start, &end);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->SetStartAndEndTimeL(start, end));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <float or False> s60_calendar_Entry::alarm()
 * Returns alarm time, or False if there's no alarm.
 */
PHP_METHOD(s60_calendar_Entry, alarm)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetAlarmL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}


/**
 * s60_calendar_Entry::set_alarm([<float> alarm_time=0])
 * Sets the alarm time (or clears the alarm if given 0)
 */
PHP_METHOD(s60_calendar_Entry, set_alarm)
{
	double alarm = 0;
	GET_METHOD_ARGS_1(s60_calendar_Entry, "O|d", &alarm);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, {
		if (alarm == 0) {
			self->entry->ClearAlarmL();
		} else {
			self->entry->SetAlarmL(alarm);
		}
	});
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}


/**
 * <float or bool> s60_calendar_Entry::crossed_out()
 * For TODO entries:
 *   Returns time when entry was crossed out, or False
 *   if it hasn't been crossed out.
 * For other entries (in S60 version 2.x only):
 *   Returns if the entry has been crossed out.
 */
PHP_METHOD(s60_calendar_Entry, crossed_out)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetCrossedOutL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}


/**
 * s60_calendar_Entry::set_crossed_out([<float> crossed_out=0])
 * For TODO entries:
 *   Sets the entry as crossed out at the given time,
 *   or if given 0, sets the entry as not crossed out.
 * For other entries (in S60 version 2.x only):
 *   Sets the entry as crossed / not crossed out.
 */
PHP_METHOD(s60_calendar_Entry, set_crossed_out)
{
	double crossed_out = 0;
	GET_METHOD_ARGS_1(s60_calendar_Entry, "O|d", &crossed_out);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, {
		if (crossed_out == 0) {
			self->entry->ClearCrossedOutL();
		} else {
			self->entry->SetCrossedOutL(crossed_out);
		}
	});
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}

/**
 * <s60_calendar_TodoList> s60_calendar_Entry::todo_list()
 * Returns the todo list to which this entry belongs.
 * Only valid for TODO entries.
 */
PHP_METHOD(s60_calendar_Entry, todo_list)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetTodoListL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}


/**
 * s60_calendar_Entry::set_todo_list(<s60_calendar_TodoList> todolist)
 * Sets this entry's todo list.
 * Only valid for TODO entries.
 */
PHP_METHOD(s60_calendar_Entry, set_todo_list)
{
	zval* todolist;
	GET_METHOD_ARGS_2(s60_calendar_Entry, "OO", &todolist, CLASS_ENTRY(s60_calendar_TodoList));
	CalendarTodoList* tl = GET_STRUCT(todolist, s60_calendar_TodoList)->todolist;
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->SetTodoListL(tl));
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}


/**
 * <int> s60_calendar_Entry::priority()
 * Priority is a value between 0-255
 */
PHP_METHOD(s60_calendar_Entry, priority)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetPriorityL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}


/**
 * s60_calendar_Entry::set_priority(<int> priority)
 * Priority is a value between 0-255
 */
PHP_METHOD(s60_calendar_Entry, set_priority)
{
	long priority;
	GET_METHOD_ARGS_1(s60_calendar_Entry, "Ol", &priority);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->SetPriorityL(priority));
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}

/**
 * <array or False> s60_calendar_Entry::repeat()
 * Returns entry's repeat rules as a complex associative array, or
 * False if no repeat has been set.
 *
 * The array always contains the following key:
 * "type" => repeat type, see below for the alternatives
 *
 * Optionally the following keys may be specified:
 * "start"      => repeat start date. This must be specified unless type is
 *                 "no_repeat"
 * "end"        => repeat end date. If unspecified or 0, repeat
 *                 will be forever.
 * "interval"   => repeat interval. Default = 1. If this is
 *                 for example 3 and repeat type is daily repeat, it
 *                 means entry is repeated every 3 days.
 * "rdates"     => an array of specific times (unix time) on which 
 *                 the entry should be repeated. Only on S60 version 3.x.
 * "exceptions" => an array of specific times (unix time) which are
 *                 exceptions to the repeat rule. On S60 version 3.x
 *                 the times should be at the same time when some instance
 *                 starts, with second precision. On S60 version 2.x it
 *                 is sufficient that the times be at the same days as the
 *                 instances that should be "removed".
 *
 * The repeat type may be one of the following:
 *
 * "no_repeat"
 * This clears the repeat. However, on S60 version 3.x one can still
 * have an rdates array to have the entry repeat on specific dates, e.g.
 * set_repeat(array("type"=>"no_repeat",
 *                  "rdates"=>array(time(), time() + $X)));
 *
 * "daily"
 * Entry will be repeated daily.
 * 
 * "weekly"
 * Entry will be repeated weekly.
 *
 * "monthly_by_dates"
 * Entry will be repeated monthly. With this type one can specify
 * for example that the entry is repeated on third and 14th day of every month.
 * This has the following extra key:
 *   "days" => an array of numbers representing days of month. 0 = first day of month.
 *
 * "monthly_by_days"
 * Entry will be repeated monthly. With this type one can specify
 * for example that the entry is repeated on second tuesday of every month
 * and last saturday of every month.
 * This has the following extra key:
 *   "days" => an array of associative arrays with the following keys:
 *      "day"  => weekday of repeat, 0 = Monday
 *      "week" => week of repeat, 1 meaning "first monday" if "day" = 0
 *                And -1 meaning "last monday".
 * 
 * "yearly_by_date"
 * Entry will be repeated yearly on repeat start date.
 *
 * "yearly_by_day"
 * Entry will be repeated yearly. With this type one can specify
 * for example that the entry is repeated on second tuesday of June.
 * This has the following extra key:
 *   "days" => an associative array with the following keys:
 *      "day"   => weekday of repeat, 0 = Monday
 *      "week"  => week of repeat, 1 meaning "first monday" if "day" = 0
 *                 And -1 meaning "last monday".
 *      "month" => month of repeat, 0 = January
 *
 * 
 * Some examples of repeat rules:
 * array("type"=>"daily", "start"=>time(), "interval"=>2);
 *  - repeat on every second day, starting today, repeat forever
 *
 * array("type"=>"monthly_by_days", "start"=>time(), "end"=>time() + 3600*24*30*3
 *       "days"=>array(array("week"=>2, "day"=>4)));
 *  - repeat on every month's second friday, for 3 months.
 *
 * Further examples of the repeat rules can be seen in source files:
 * /php-5.1.4/ext/s60_calendar/tests/repeat.phpt
 * /php-5.1.4/ext/s60_calendar/tests/eka2repeat.phpt
 */
PHP_METHOD(s60_calendar_Entry, repeat)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetRepeatL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}



static CArrayFixSeg<TReal>* parseRealArrayL(zval** zvalue) {
	if (Z_TYPE_PP(zvalue) != IS_ARRAY) {
		User::Leave(KErrGeneral);
	}
	HashTable *arr_hash = Z_ARRVAL_PP(zvalue);
	int elements = zend_hash_num_elements(arr_hash);
	CArrayFixSeg<TReal>* realArray = 0;
    if (elements > 0) {
		TRAPD(error, 
			realArray = new (ELeave) CArrayFixSeg<TReal>(elements));
		User::LeaveIfError(error);
		
		zval **data;
		ARRAY_FOREACH(pointer, arr_hash, data) {
			if (Z_TYPE_PP(data) != IS_LONG && Z_TYPE_PP(data) != IS_DOUBLE) {
				delete realArray;
				User::Leave(KErrGeneral);
			}
			realArray->AppendL(Z_DVAL_SAFE(**data));
		}
    }
    return realArray;
}



/**
 * s60_calendar_Entry::set_repeat([<array or False> repeat=False])
 * Sets entry's repeat rules. Array given should follow the
 * same rules as specified in s60_calendar_Entry::repeat()
 * documentation. Giving False or no parameters unsets the repeat.
 */
PHP_METHOD(s60_calendar_Entry, set_repeat)
{
	zval* repeat = 0;
	GET_METHOD_ARGS_1(s60_calendar_Entry, "O|z", &repeat);
	ASSERT_ENTRY_OPEN;
	
	if (repeat && Z_TYPE_P(repeat) != IS_ARRAY && (Z_TYPE_P(repeat) != IS_BOOL || Z_BVAL_P(repeat))) {
		RETURN_ERROR("Parameter must be an array or False");	
	}
	
	if (repeat && Z_TYPE_P(repeat) == IS_BOOL && !Z_BVAL_P(repeat)) {
		repeat = 0;	
	}
	
	TInt error = KErrNone;
	
	if (!repeat) {
		TRAP(error, self->entry->ClearRepeatL());
	} else {
		zval** zvalue = 0;
		
		int type = 0;
		if (zend_hash_find(Z_ARRVAL_P(repeat), KRepeatType, sizeof(KRepeatType), (void**)&zvalue) == SUCCESS) {
			if (Z_TYPE_PP(zvalue) != IS_STRING) {
				RETURN_ERROR("Repeat type must be string");	
			}
			TPtrC8 valuePtr((TUint8*)Z_STRVAL_PP(zvalue), Z_STRLEN_PP(zvalue));
			if (0 == valuePtr.Compare(KNoRepeat)) {
				type = NO_REPEAT;
			} else if (0 == valuePtr.Compare(KDaily)) {
				type = DAILY_REPEAT;
			} else if (0 == valuePtr.Compare(KWeekly)) {
				type = WEEKLY_REPEAT;
			} else if (0 == valuePtr.Compare(KMonthlyByDates)) {
				type = MONTHLY_BY_DATES_REPEAT;
			} else if (0 == valuePtr.Compare(KMonthlyByDays)) {
				type = MONTHLY_BY_DAYS_REPEAT;
			} else if (0 == valuePtr.Compare(KYearlyByDate)) {
				type = YEARLY_BY_DATE_REPEAT;
			} else if (0 == valuePtr.Compare(KYearlyByDay)) {
				type = YEARLY_BY_DAY_REPEAT;
			} else {
				RETURN_ERROR("Illegal repeat type");
			}
		} else {
			RETURN_ERROR("No repeat type given");	
		}
		
		TReal startDate = 0;
		TReal endDate = 0;
		TInt interval = 1;
		TBool eternalRepeat = EFalse;
		HashTable *repeatDays = 0;
		
		if (type != NO_REPEAT) {
			if (zend_hash_find(Z_ARRVAL_P(repeat), KStartDate, sizeof(KStartDate), (void**)&zvalue) == SUCCESS) {
				if (Z_TYPE_PP(zvalue) != IS_LONG && Z_TYPE_PP(zvalue) != IS_DOUBLE) {
					RETURN_ERROR("Start date must be long or double");
				}
				startDate = Z_DVAL_SAFE(**zvalue);
			} else {
				RETURN_ERROR("No start date given");	
			}
			
			if (zend_hash_find(Z_ARRVAL_P(repeat), KEndDate, sizeof(KEndDate), (void**)&zvalue) == SUCCESS) {
				if (Z_TYPE_PP(zvalue) != IS_LONG && Z_TYPE_PP(zvalue) != IS_DOUBLE) {
					RETURN_ERROR("End date must be long or double");
				}
				endDate = Z_DVAL_SAFE(**zvalue);
			}
			if (!endDate) eternalRepeat = ETrue;
			
			if (zend_hash_find(Z_ARRVAL_P(repeat), KInterval, sizeof(KInterval), (void**)&zvalue) == SUCCESS) {
				if (Z_TYPE_PP(zvalue) != IS_LONG || Z_LVAL_PP(zvalue) <= 0) {
					RETURN_ERROR("Repeat interval must be long and greater than 0");	
				}
		        interval = Z_LVAL_PP(zvalue);
			}
			
			if (zend_hash_find(Z_ARRVAL_P(repeat), KRepeatDays, sizeof(KRepeatDays), (void**)&zvalue) == SUCCESS) {
				if (Z_TYPE_PP(zvalue) != IS_ARRAY) {
					RETURN_ERROR("Repeat days must be an array");	
				}
		        repeatDays = Z_ARRVAL_PP(zvalue);
			}
		}
		
		CArrayFixSeg<TReal>* exceptionArray = 0;
		if (zend_hash_find(Z_ARRVAL_P(repeat), KExceptions, sizeof(KExceptions), (void**)&zvalue) == SUCCESS) {
			TRAP(error, exceptionArray = parseRealArrayL(zvalue));
			if (error != KErrNone) {
				if (error == KErrGeneral) {
					RETURN_ERROR("Exceptions must be an array of longs or doubles");
				} else {
					RETURN_SYMBIAN_ERROR(error);
				}
			}
		}
		
		CArrayFixSeg<TReal>* rdateArray = 0;
		if (zend_hash_find(Z_ARRVAL_P(repeat), KRDates, sizeof(KRDates), (void**)&zvalue) == SUCCESS) {
			TRAP(error, rdateArray = parseRealArrayL(zvalue));
			if (error != KErrNone) {
				if (error == KErrGeneral) {
					RETURN_ERROR("RDates must be an array of longs or doubles");
				} else {
					RETURN_SYMBIAN_ERROR(error);
				}
			}
		}
		
		// no repeat => clear previous repeat
		// (even if there's "no repeat" one can have rdates and exceptions
		// on EKA2 platform. EKA1 shall return immediately from SetRepeatL)
		if (type == NO_REPEAT) {
			TRAP(error, self->entry->ClearRepeatL());
			RETURN_IF_SYMBIAN_ERROR(error);
		}
		
		TRAP(error, self->entry->SetRepeatL(type, startDate, endDate,
				eternalRepeat, interval, exceptionArray, rdateArray, repeatDays));
		delete exceptionArray;
		
		if (error != KErrNone) {
			TRAPD(error2, self->entry->ClearRepeatL());
		}
	}
	
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}


/**
 * <int> s60_calendar_Entry::status()
 * TODO entries may have one of S60_CALENDAR_STATUS_CANCELLED, S60_CALENDAR_STATUS_TODO_*
 * Other entires may have one of S60_CALENDAR_STATUS_CANCELLED, S60_CALENDAR_STATUS_TENTATIVE,
 * S60_CALENDAR_STATUS_CONFIRMED.
 * Returns S60_CALENDAR_STATUS_NULL if status has not been set.
 *
 * Only supported in S60 version 3.x.
 */
PHP_METHOD(s60_calendar_Entry, status)
{
	GET_METHOD_ARGS_0(s60_calendar_Entry);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->GetStatusL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}


/**
 * s60_calendar_Entry::set_status(<int> status)
 * TODO entries may have one of S60_CALENDAR_STATUS_CANCELLED, S60_CALENDAR_STATUS_TODO_*
 * Other entires may have one of S60_CALENDAR_STATUS_CANCELLED, S60_CALENDAR_STATUS_TENTATIVE,
 * S60_CALENDAR_STATUS_CONFIRMED.
 *
 * Only supported in S60 version 3.x.
 */
PHP_METHOD(s60_calendar_Entry, set_status)
{
	long status;
	GET_METHOD_ARGS_1(s60_calendar_Entry, "Ol", &status);
	ASSERT_ENTRY_OPEN;
	TRAPD(error, self->entry->SetStatusL(status));
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}



/**********************************************************************
 * Class s60_calendar_Instance
 * This represents a specific instance of a calendar entry.
 * The entry() method can be used to retreive the entry.
 **********************************************************************/

BEGIN_CLASS_FUNCTIONS(s60_calendar_Instance)
	PHP_ME(s60_calendar_Instance, __construct, NULL, ZEND_ACC_PRIVATE)
	PHP_ME(s60_calendar_Instance, entry, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Instance, date, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_Instance, remove, NULL, ZEND_ACC_PUBLIC)
END_CLASS_FUNCTIONS()

DEFINE_OBJECT_FREE_FUNCTION(s60_calendar_Instance, {
	SAFE_CLEAR_REFERENCE(self->instance);
})
DEFINE_OBJECT_NEW_FUNCTION(s60_calendar_Instance, {
	self->instance = new CalendarInstance;
})
DEFINE_CLASS_REGISTER_FUNCTION(s60_calendar_Instance)

#define ASSERT_INSTANCE_OPEN { if (!self->instance) { RETURN_ERROR("Instance not open"); } }

PHP_METHOD(s60_calendar_Instance, __construct) { }

/**
 * <s60_calendar_Entry> s60_calendar_Instance::entry()
 */
PHP_METHOD(s60_calendar_Instance, entry)
{
	GET_METHOD_ARGS_0(s60_calendar_Instance);
	ASSERT_INSTANCE_OPEN;
	TRAPD(error, self->instance->GetEntryL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <float> s60_calendar_Instance::date()
 */
PHP_METHOD(s60_calendar_Instance, date)
{
	GET_METHOD_ARGS_0(s60_calendar_Instance);
	ASSERT_INSTANCE_OPEN;
	TRAPD(error, self->instance->GetDateL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * s60_calendar_Instance::remove([<int> modifier=S60_CALENDAR_REMOVE_THIS])
 * Removes the instance. If the modifier is
 *  S60_CALENDAR_REMOVE_PAST   : also removes past instances
 *  S60_CALENDAR_REMOVE_FUTURE : also removes future instances
 *
 * Note: The removal is done by modifying the underlying entry. For REMOVE_THIS,
 * an exception is added to this instances's date. For REMOVE_PAST, the
 * entry's start date is modified and for REMOVE_FUTURE the entry's
 * end date is modified.
 */
PHP_METHOD(s60_calendar_Instance, remove)
{
	long modifier = REMOVE_THIS;
	GET_METHOD_ARGS_1(s60_calendar_Instance, "O|l", &modifier);
	ASSERT_INSTANCE_OPEN;
	
	if (modifier != REMOVE_THIS && modifier != REMOVE_FUTURE && modifier != REMOVE_PAST) {
		RETURN_ERROR("Illegal remove modifier");	
	}
	
	TRAPD(error, self->instance->RemoveL(modifier));
	RETURN_IF_SYMBIAN_ERROR(error);
	SAFE_CLEAR_REFERENCE(self->instance);
	RETURN_TRUE;
}

/**********************************************************************
 * Class s60_calendar_TodoList
 * A list of TODO entries. Only supported in Series 60 version 2.x.
 **********************************************************************/

BEGIN_CLASS_FUNCTIONS(s60_calendar_TodoList)
	PHP_ME(s60_calendar_TodoList, __construct, NULL, ZEND_ACC_PRIVATE)
	PHP_ME(s60_calendar_TodoList, todos, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_TodoList, id, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_TodoList, name, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_TodoList, set_name, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_calendar_TodoList, remove, NULL, ZEND_ACC_PUBLIC)
END_CLASS_FUNCTIONS()

DEFINE_OBJECT_FREE_FUNCTION(s60_calendar_TodoList, {
	SAFE_CLEAR_REFERENCE(self->todolist);
})
DEFINE_OBJECT_NEW_FUNCTION(s60_calendar_TodoList, {
	self->todolist = new CalendarTodoList;
})
DEFINE_CLASS_REGISTER_FUNCTION(s60_calendar_TodoList)

#define ASSERT_TODOLIST_OPEN { if (!self->todolist) { RETURN_ERROR("Todo list not open"); } }

PHP_METHOD(s60_calendar_TodoList, __construct) { }

/**
 * s60_calendar_TodoList::remove()
 * Removing a todo list does not remove the entries belonging
 * to the list. But the entires will no longer belong to any
 * list after the removal.
 */
PHP_METHOD(s60_calendar_TodoList, remove)
{
	GET_METHOD_ARGS_0(s60_calendar_TodoList);
	ASSERT_TODOLIST_OPEN;
	TRAPD(error, self->todolist->RemoveL());
	RETURN_IF_SYMBIAN_ERROR(error);
	SAFE_CLEAR_REFERENCE(self->todolist);
	RETURN_TRUE;
}

/**
 * <array of s60_calendar_Instance> s60_calendar_TodoList::todos()
 * Returns a list of TODO instances belonging to this todo list.
 */
PHP_METHOD(s60_calendar_TodoList, todos)
{
	GET_METHOD_ARGS_0(s60_calendar_TodoList);
	ASSERT_TODOLIST_OPEN;
	TRAPD(error, self->todolist->GetTodosL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <int> s60_calendar_TodoList::id()
 * Returns the unique id of this todo list, to be used with
 * <s60_calendar_Database>::get_todo_list()
 */
PHP_METHOD(s60_calendar_TodoList, id)
{
	GET_METHOD_ARGS_0(s60_calendar_TodoList);
	ASSERT_TODOLIST_OPEN;
	TRAPD(error, self->todolist->GetIdL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <string> s60_calendar_TodoList::name()
 */
PHP_METHOD(s60_calendar_TodoList, name)
{
	GET_METHOD_ARGS_0(s60_calendar_TodoList);
	ASSERT_TODOLIST_OPEN;
	TRAPD(error, self->todolist->GetNameL(return_value));
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * s60_calendar_TodoList::set_name(<string> name)
 */
PHP_METHOD(s60_calendar_TodoList, set_name)
{
	char *name;
	int name_len;
	GET_METHOD_ARGS_2(s60_calendar_TodoList, "Os", &name, &name_len);
	ASSERT_TODOLIST_OPEN;
	TRAPD(error, self->todolist->SetNameL(name, name_len));
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}



} /* extern "C" */

