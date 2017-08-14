#ifndef PHP_S60_CALENDAR_H
#define PHP_S60_CALENDAR_H

PHP_MINIT_FUNCTION(s60_calendar);

PHP_FUNCTION(s60_calendar_open);

PHP_METHOD(s60_calendar_Database, __construct);
PHP_METHOD(s60_calendar_Database, open);
PHP_METHOD(s60_calendar_Database, close); //
PHP_METHOD(s60_calendar_Database, get_entry);
PHP_METHOD(s60_calendar_Database, add_entry);
PHP_METHOD(s60_calendar_Database, entries);
PHP_METHOD(s60_calendar_Database, find_instances_month);
PHP_METHOD(s60_calendar_Database, find_instances_day);
PHP_METHOD(s60_calendar_Database, find_instances_range); //buggy
PHP_METHOD(s60_calendar_Database, get_todo_list);
PHP_METHOD(s60_calendar_Database, add_todo_list);
PHP_METHOD(s60_calendar_Database, todo_lists);

PHP_METHOD(s60_calendar_Entry, __construct);
PHP_METHOD(s60_calendar_Entry, remove);
PHP_METHOD(s60_calendar_Entry, commit);
PHP_METHOD(s60_calendar_Entry, id);
PHP_METHOD(s60_calendar_Entry, type);
PHP_METHOD(s60_calendar_Entry, last_modified);
PHP_METHOD(s60_calendar_Entry, content);
PHP_METHOD(s60_calendar_Entry, set_content);
PHP_METHOD(s60_calendar_Entry, location);
PHP_METHOD(s60_calendar_Entry, set_location);
PHP_METHOD(s60_calendar_Entry, replication);
PHP_METHOD(s60_calendar_Entry, set_replication);
PHP_METHOD(s60_calendar_Entry, start_time);
PHP_METHOD(s60_calendar_Entry, end_time);
PHP_METHOD(s60_calendar_Entry, set_start_end_time); //
PHP_METHOD(s60_calendar_Entry, alarm);
PHP_METHOD(s60_calendar_Entry, set_alarm);
PHP_METHOD(s60_calendar_Entry, crossed_out);
PHP_METHOD(s60_calendar_Entry, set_crossed_out);
PHP_METHOD(s60_calendar_Entry, todo_list);      //
PHP_METHOD(s60_calendar_Entry, set_todo_list);  //
PHP_METHOD(s60_calendar_Entry, priority);      //
PHP_METHOD(s60_calendar_Entry, set_priority);  //
PHP_METHOD(s60_calendar_Entry, repeat);      //
PHP_METHOD(s60_calendar_Entry, set_repeat);  //
PHP_METHOD(s60_calendar_Entry, status);      //
PHP_METHOD(s60_calendar_Entry, set_status);  //

PHP_METHOD(s60_calendar_TodoList, __construct);
PHP_METHOD(s60_calendar_TodoList, todos); //
PHP_METHOD(s60_calendar_TodoList, id);
PHP_METHOD(s60_calendar_TodoList, name);
PHP_METHOD(s60_calendar_TodoList, set_name);
PHP_METHOD(s60_calendar_TodoList, remove);

PHP_METHOD(s60_calendar_Instance, __construct);
PHP_METHOD(s60_calendar_Instance, entry);
PHP_METHOD(s60_calendar_Instance, date);
PHP_METHOD(s60_calendar_Instance, remove); //

extern zend_module_entry s60_calendar_module_entry;
#define phpext_s60_calendar_ptr &s60_calendar_module_entry

#endif

