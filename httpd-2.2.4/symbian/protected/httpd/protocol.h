#ifndef httpd_protocol_h
#define httpd_protocol_h
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


#define HTTPD_PROTOCOL_VERSION_MAJOR 1
#define HTTPD_PROTOCOL_VERSION_MINOR 1


namespace Httpd
{
    enum TProtocolVersion
        {
        KProtocolVersionMajor = HTTPD_PROTOCOL_VERSION_MAJOR,
        KProtocolVersionMinor = HTTPD_PROTOCOL_VERSION_MINOR,
        };
    

    enum TRequests
        {
        /* Do NOT EVER CHANGE THE ORDER or add anything BUT TO THE END! */

        EHttpdReqStart,
        EHttpdReqStop,
        EHttpdReqRestart,

        EHttpdReqGetState,
        EHttpdReqGetStateAsync,
        EHttpdReqGetStateAsyncCancel,

        EHttpdReqStart2,

        EHttpdReqSetSettings,
        EHttpdReqGetSettings,
        EHttpdReqGetCurrentSettings,
        };
}

#endif
