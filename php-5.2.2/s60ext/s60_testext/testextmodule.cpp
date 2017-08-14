extern "C" {

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "testextmodule.h"
#include "symbian_utils.h"

} // Extern "C"

#include <e32base.h>

extern "C" {

/* Function entry list */
static function_entry testext_functions[] = {
    PHP_FE(open_mysql_connection, NULL)
    PHP_FE(active_scheduler_wait, NULL)
    PHP_FE(symbian_log, NULL)
    {NULL, NULL, NULL}
};

/* Module entry */
zend_module_entry testext_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_TESTEXTNAME,
    testext_functions,
    PHP_MINIT(s60_testext),
    PHP_MSHUTDOWN(s60_testext),
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_TESTEXT_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

/* PHP-INI entries */
PHP_INI_BEGIN()
PHP_INI_ENTRY("s60_testext.logging", "1", PHP_INI_ALL, NULL)
PHP_INI_ENTRY("s60_testext.clear_log_on_startup", "1", PHP_INI_ALL, NULL)
PHP_INI_END()

#ifdef COMPILE_DL_TESTEXT
ZEND_GET_MODULE(testext)
#endif

/* Function definitions */

/** Module init function */
PHP_MINIT_FUNCTION(s60_testext)
{
	REGISTER_INI_ENTRIES();
	
	if (INI_BOOL("s60_testext.clear_log_on_startup")) {
		php_symbian_clear_log();
	}
	
	if (INI_BOOL("s60_testext.logging")) {
		php_symbian_log("STARTING s60_testext.\n");
	}
	
	return SUCCESS;
}

/** Module shutdown function */
PHP_MSHUTDOWN_FUNCTION(s60_testext)
{
	if (INI_BOOL("s60_testext.logging")) {
		php_symbian_log("SHUTTING DOWN s60_testext.\n");
	}

	UNREGISTER_INI_ENTRIES();
	
	return SUCCESS;
}

PHP_FUNCTION(open_mysql_connection)
{
	// Not implemented
    RETURN_STRING("MySQL connection not really opened", 1);
}

PHP_FUNCTION(active_scheduler_wait)
{
	TRAPD(err, {
		CActiveSchedulerWait* activeSchedulerWait = new (ELeave) CActiveSchedulerWait();
		CleanupStack::PushL(activeSchedulerWait);
    	php_symbian_log("Active scheduler wait about to be called...\n");
		// This will wait forever unless something causes a leave or a panic
		activeSchedulerWait->Start();
	    php_symbian_log("Returned from active scheduler wait.\n", 1);
	});
	if(err != KErrNone) {
		php_symbian_log("Trapped leave with code %d in PHP_FUNCTION: active_scheduler_wait.\n", err);
	}
}

PHP_FUNCTION(symbian_log)
{
	// Not implemented.  Make a call to php_symbian_log()
}

} // Extern "C"
