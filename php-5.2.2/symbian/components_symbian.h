#ifndef components_symbian_h
#define components_symbian_h
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
 */

/*	
 *
 * File defines all internal functions that we are going to build into 
 * Symbian binary. 
 *
 * This file should be compatible with any MMP and zend_config-symbian.h
 */
#define HAVE_BUNDLED_PCRE 1
#define HAVE_CURL		  1
#define HAVE_DATE		  1
#define HAVE_DOM          1
#define HAVE_GLOB         1
#define HAVE_LIBGD		  1
#define HAVE_LIBXML		  1
#define HAVE_MBSTRING 	  1
#define HAVE_MYSQL		  1
#define HAVE_MYSQLI		  1
#define HAVE_PCRE		  1
#define HAVE_PHP_SESSION  1
#define HAVE_SIMPLEXML    1
#define HAVE_STRFTIME	  1
#define HAVE_TOKENIZER    1
#define HAVE_XML          1
#define HAVE_XSL          1
#define HAVE_ZLIB         1

/*
 * S60 specific modules.
 *
 * Define HAVE_S60_EXTENSION_SUPPORT if you have any of these modules 
 * in your build. This is required in order to define correct capabilities. 
 * See SAPI MMP's and modules.mmp for more information.   
 */
#define HAVE_S60_EXTENSION_SUPPORT 1

/*
 * Actual modules are defined here. 
 */
#define HAVE_S60_CALENDAR 1
#define HAVE_S60_CONTACTS 1
#define HAVE_S60_INBOX    1
#define HAVE_S60_LOCATION 1
#define HAVE_S60_LOG      1
#define HAVE_S60_PATHINFO 1
#define HAVE_S60_SYSINFO  1


/* Testing extension */
#define HAVE_S60_TESTEXT 0

#endif
