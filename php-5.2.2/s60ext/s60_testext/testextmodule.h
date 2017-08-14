#ifndef __PHP_TESTEXTMODULE_H__
#define __PHP_TESTEXTMODULE_H__

/* Module information constants */
#define PHP_TESTEXT_VERSION "1.0"
#define PHP_TESTEXTNAME "s60_test_extension"

/* Function declarations */
PHP_MINIT_FUNCTION(s60_testext);
PHP_MSHUTDOWN_FUNCTION(s60_testext);

PHP_FUNCTION(open_mysql_connection);
PHP_FUNCTION(active_scheduler_wait);
PHP_FUNCTION(symbian_log);

/* Handles to the module entry structure */
extern zend_module_entry testext_module_entry;
#define phpext_testext_ptr &testext_module_entry

#endif

