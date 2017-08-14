#ifndef mod_python_symbian_config_symbian_mod_python_config_h
#define mod_python_symbian_config_symbian_mod_python_config_h
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


#include "symbian_mod_python_builtin.h"

/***************************************************************************
 *
 * Platform Identifiers.
 *
 * These have been taken from the document 
 *
 *     S60 Platform:
 *     Identification Codes
 *     Version 2.0
 *     November 2, 2007
 *
 * Available at http://www.forum.nokia.com.
 */

#define PlatID_S60_3rd_Ed     0X101F7961
#define PlatID_S60_3rd_Ed_FP1 0x102032BE

/*
 * UID2 for static interface DLL.
 */
#define UID2_DLL 0x1000008d


/*
 * Version of mod_python port.
 */
#define VERSION_MOD_PYTHON_MAJOR      1
#define VERSION_MOD_PYTHON_MINOR      0
#define VERSION_MOD_PYTHON_CORRECTION 3


/**************************************************************************/

#include "configure_symbian_mod_python.h"

/***************************************************************************
 *
 * Here follows defaults.
 *
 * Do NOT edit these, but look into configure.h for how to change them
 * into something else.
 *
 **************************************************************************/

/*
 * Platform UID.
 */
#if !defined(PlatID_S60_DEFAULT)
#define PlatID_S60_DEFAULT PlatID_S60_3rd_Ed
#endif


/*
 * Package UIDs.
 */
#if !defined(UID_PKG_MOD_PYTHON)
/* If the UID is ever changes, then change MOD_PYTHON_BACKUP_TARGET 
 * below as well.
 */
#define UID_PKG_MOD_PYTHON          0xe1666358
#else
#define UID_PKG_MOD_PYTHON_WAS_DEFINED
#endif

#if !defined(MOD_PYTHON_BACKUP_SOURCE)
#define MOD_PYTHON_BACKUP_SOURCE "backup_registration.xml"
#endif

#if !defined(MOD_PYTHON_BACKUP_TARGET)
#if defined(UID_PKG_MOD_PYTHON_WAS_DEFINED)
#error If you define UID_PKG_MOD_PYTHON, then you must define MOD_PYTHON_BACKUP_TARGET
#else
#define MOD_PYTHON_BACKUP_TARGET "!:\private\10202D56\import\packages\e1666358\backup_registration.xml"
#endif
#endif


/***************************************************************************
 *
 * Capabilities
 */

#if defined(MWS_WITH_DEVCERT)
#warning MWS_WITH_DEVCERT no longer carries any meaning.
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

#if !defined(MWS_DLL_CAPABILITIES)

#if defined(MWS_HTTPDS_CAPABILITIES)

#define MWS_DLL_CAPABILITIES MWS_HTTPDS_CAPABILITIES

#else

#define MWS_DLL_CAPABILITIES\
    LocalServices\
    NetworkServices\
    ReadUserData\
    WriteUserData\
    UserEnvironment\
    \
    Location

#endif /* MWS_HTTPD_CAPABILITIES */

#endif /* MWS_DLL_CAPABILITIES */

#endif
