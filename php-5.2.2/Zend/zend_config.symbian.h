/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2007 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef ZEND_CONFIG_SYMBIAN_H
#define ZEND_CONFIG_SYMBIAN_H

#include <e32def.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>
#include <sys/select.h>

#define PHP_OS  	"Symbian"
#define PHP_UNAME  	"Symbian"

#define CONFIGURATION_FILE_PATH "php.ini"
#define PEAR_INSTALLDIR "c:\\php5\\pear"
#define PHP_BINDIR "c:\\php5"
#define PHP_CONFIG_FILE_PATH "c:\\data\\php"
#define PHP_CONFIG_FILE_SCAN_DIR ""
#define PHP_DATADIR "c:\\php5"
#define PHP_EXTENSION_DIR "c:\\php5"
#define PHP_INCLUDE_PATH	".;c:\\php5\\pear"
#define PHP_LIBDIR "c:\\php5"
#define PHP_LOCALSTATEDIR "c:\\php5"
#define PHP_PREFIX "c:\\php5"
#define PHP_SYSCONFDIR "c:\\php5"
#define PHP_SHLIB_SUFFIX "dll"

#define PHP_TMPDIR "c:\\php5\\tmp"

/*
MOD_PHP should be threadsafe. 
*/
#define THREADSAFE 1 
//#define ZTS 1

/*
	Open C has pthreads.
*/
#define PTHREADS 1

/* Enable / Disable crypt() function (default: enabled) */
#define HAVE_CRYPT 1
#define PHP_STD_DES_CRYPT 1
#define PHP_EXT_DES_CRYPT 0
#define PHP_MD5_CRYPT 1
#define PHP_BLOWFISH_CRYPT 0

/* PHP Runtime Configuration */
#define PHP_URL_FOPEN 1
#define PHP_SAFE_MODE 0
#define MAGIC_QUOTES 0
#define USE_CONFIG_FILE 1
#define DEFAULT_SHORT_OPEN_TAG "1"

/* Use PHP's own regex implementation.
 */
#define REGEX 1

#define ZEND_DEBUG 0

#define LIBZEND_EXPORTS

#include "../symbian/config/openc.h"
#include "../symbian/components_symbian.h"

typedef TUint32 ulong;

#define zend_isinf(a)	isinf(a)
#define zend_finite(x)	finite(x)
#define zend_isnan(x)	isnan(x)

#ifdef LIBZEND_EXPORTS
#	define ZEND_API EXPORT_C
#else
#	define ZEND_API IMPORT_C
#endif

#define zend_sprintf sprintf

#if defined( __ARMCC__ )
#define alloca g_malloc
#endif

#define ZEND_DLEXPORT		EXPORT_C
#define ZEND_DLIMPORT		IMPORT_C

#endif /* ZEND_CONFIG_SYMBIAN_H */

