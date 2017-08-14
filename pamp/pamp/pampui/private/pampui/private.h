#ifndef pampui_private_h
#define pampui_private_h
/* Copyright 2006 Nokia Corporation
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

#include <e32base.h>
#include <pampui.rsg>
#include <pampui.mbg>
#include "pampui/private.hrh"
#include "pampui/guidspampui.hrh"


namespace PampUi
{
    const TUid KUid3 = { UID3_PAMPUI };

    enum TCommands
        {
        ECmdStart           = EPampCmdStart,
        ECmdStartPamp       = EPampCmdStartPamp,
        ECmdStartHttpd      = EPampCmdStartHttpd,
        ECmdStartMysqld     = EPampCmdStartMysqld,
        ECmdStop            = EPampCmdStop, 
        ECmdStopPamp        = EPampCmdStopPamp, 
        ECmdStopHttpd       = EPampCmdStopHttpd,   
        ECmdStopMysqld      = EPampCmdStopMysqld,   
        ECmdSettings        = EPampCmdSettings,
        ECmdSettingsHttpd   = EPampCmdSettingsHttpd,
        ECmdSettingsMysqld  = EPampCmdSettingsMysqld,
        ECmdStartWLAN       = EPampCmdStartWLAN,
        ECmdCloseWLAN       = EPampCmdCloseWLAN,
        ECmdAttachWLAN      = EPampCmdAttachWLAN,
        ECmdArguments       = EPampCmdArguments,
        ECmdArgumentsHttpd  = EPampCmdArgumentsHttpd,
        ECmdArgumentsMysqld = EPampCmdArgumentsMysqld,
        };
}

#endif
