#ifndef mod_php_symbian_config_config_h
#define mod_php_symbian_config_config_h
/* Copyright 2008 Nokia Corporation
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
 */


/*
 * Version of mod_php port.
 *
 * NOTE: The major number is always 1, and the minor 0.
 */
#define VERSION_MOD_PHP_CORRECTION 2
#define VERSION_PHP_CORRECTION     2


/**************************************************************************/

#include "configure_symbian_php.h"

/***************************************************************************
 *
 * Here follows defaults.
 *
 * Do NOT edit these, but look into configure.h for how to change them
 * into something else.
 */

/*
 * Package UIDs.
 */
#if !defined(UID_PKG_MOD_PHP)
#define UID_PKG_MOD_PHP 0xE8492A36
#endif

#if !defined(UID_PKG_PHP)
#define UID_PKG_PHP     0xE8492A37
#endif


/*
 * This define specifies the capabilities of all DLL components.
 */
#if defined(PHP_WITH_DEVCERT)
#warning PHP_WITH_DEVCERT no longer carries any meaning.
#endif

#define MAX_OPEN_SIGNED_ONLINE_CAPABILITIES\
    LocalServices\
    NetworkServices\
    ReadUserData\
    WriteUserData\
    UserEnvironment\
    \
    Location\
    PowerMgmt\
    ProtServ\
    ReadDeviceData\
    SurroundingsDD\
    SwEvent\
    TrustedUI\
    WriteDeviceData                             


#if !defined(PHP_DLL_CAPABILITIES)

#define PHP_DLL_CAPABILITIES\
    LocalServices\
    NetworkServices\
    ReadUserData\
    WriteUserData\
    UserEnvironment\
    Location

#endif /* PHP_DLL_CAPABILITIES */

#endif
