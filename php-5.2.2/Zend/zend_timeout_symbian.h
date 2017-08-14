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
   | Authors: Mikko Heikelä <mikko.heikela@futurice.com>                  |
   +----------------------------------------------------------------------+
 */

/*
Zend timeout implementation for Symbian OS.

Author(s): Mikko Heikelä <mikko.heikela@futurice.com>
*/

#ifndef __ZEND_TIMEOUT_SYMBIAN_H__
#define __ZEND_TIMEOUT_SYMBIAN_H__

void zend_set_timeout_symbian(long seconds);

void zend_unset_timeout_symbian();

#endif
