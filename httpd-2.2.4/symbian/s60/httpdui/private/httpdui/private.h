#ifndef httpdui_private_h
#define httpdui_private_h
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
#include <httpdutil.mbg>
#include <httpdui.rsg>
#include "mws/httpd.h"
#include "mws/httpdutil.h"
#include "httpdui/private.hrh"
#include "symbian_httpd_config.h"


namespace HttpdUi
{
    const TUid KUid3 = { UID3_HTTPDUI };

    enum
        {
        KIdMainView     = KHttpdIdMainView,
        KIdSettingsView = KHttpdIdSettingsView
        };

    enum TCommands
        {
        ECmdStart      = EHttpdCmdStart,
        ECmdRestart    = EHttpdCmdRestart,
        ECmdStop       = EHttpdCmdStop,
        ECmdStartWLAN  = EHttpdCmdStartWLAN,
        ECmdCloseWLAN  = EHttpdCmdCloseWLAN,
        ECmdAttachWLAN = EHttpdCmdAttachWLAN,
        ECmdSettings   = EHttpdCmdSettings,
        ECmdReset      = EHttpdCmdReset,
        ECmdArguments  = EHttpdCmdArguments
        };
}

#endif
