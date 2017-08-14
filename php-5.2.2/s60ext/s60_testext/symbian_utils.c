#include "symbian_utils.h"

#include <stdarg.h>
#include <stdio.h>

#include "tsrm_symbian.h"

void php_symbian_log(const char* format, ...)
{
	FILE* logfile = fopen("C:\\Data\\Php\\php_symbian_log.txt", "a");
	va_list args;
	va_start(args, format);
	vfprintf(logfile, format, args);
	va_end(args);
	fclose(logfile);
}

void php_symbian_clear_log()
{
	tsrm_symbian_unlink("C:\\Data\\Php\\php_symbian_log.txt");
}
