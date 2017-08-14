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
#ifndef __HOSTRESOLVER_H__
#define __HOSTRESOLVER_H__

#ifdef  __cplusplus
extern "C" {
#endif
    
/*
 * FIXME: This function returns a pointer to a static structure that
 * FIXME: overwritten by each call.
 */

struct hostent* gethostbyname_iap( const char *name );

#ifdef  __cplusplus
}
#endif

#endif
