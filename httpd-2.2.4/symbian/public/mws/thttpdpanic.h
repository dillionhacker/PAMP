#ifndef mws_httpdpanic_h
#define mws_httpdpanic_h
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

namespace Mws
{
    /**
       Defines the panic codes of the web server.
    */
    enum THttpdPanic
        {
        /**
           A fatal error occurred at startup.
        */
        EHttpdPanicServerCreate,

        /**
           The client sent a request the server did not understand.
           Indicates a version mismatch between the server and client
           library.
        */
        EHttpdPanicBadRequest,

        /**
           The client provided an invalid or incompatible descriptor.
        */
        EHttpdPanicBadDescriptor,

        /**
           The client re-issued an asynchronous request that was
           already pending.
        */
        EHttpdPanicRequestPending,

        /**
           The server run out of some crucial resource (typically memory).
         */
        EHttpdPanicOutOfResources
        };
}

#endif
