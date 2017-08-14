#ifndef httpd_protected_h
#define httpd_protected_h
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

#include "mws/httpd.h"
#include "mws/versionhttpd.hrh"
#include "httpd/protocol.h"


namespace Httpd
{
    const TUint KVersionMajor      = VERSION_HTTPD_MAJOR;
    const TUint KVersionMinor      = VERSION_HTTPD_MINOR;
    const TUint KVersionCorrection = VERSION_HTTPD_CORRECTION;


    _LIT(KServerFileName, "HTTPDS22");
    _LIT(KServerName, "HTTPDS22");
    _LIT(KServerSemaphoreName, "49116d4d-74a8-4841-ac63-f96b859153cb");
}

#endif
