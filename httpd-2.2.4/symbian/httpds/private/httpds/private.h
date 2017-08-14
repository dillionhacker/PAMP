#ifndef httpds_private_h
#define httpds_private_h
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

#include "httpd/protected.h"
#include <e32base.h>
#include "mws/thttpdstate.h"
#include "server/mpm/symbian/mpm_default.h"


namespace HttpdS
{
    using Mws::THttpdPanic;

#define HTTPDS_DEFAULT_STACK_SIZE    DEFAULT_THREAD_STACKSIZE
#define HTTPDS_DEFAULT_MIN_HEAP_SIZE 0x100000
#define HTTPDS_DEFAULT_MAX_HEAP_SIZE 0xA00000

    enum 
    {
    KDefaultStackSize   = HTTPDS_DEFAULT_STACK_SIZE,
    KDefaultMinHeapSize = HTTPDS_DEFAULT_MIN_HEAP_SIZE,
    KDefaultMaxHeapSize = HTTPDS_DEFAULT_MAX_HEAP_SIZE
    };


    void PanicClient(const RMessage2& aMessage, TInt aPanic);

    void WriteLog(const TDesC& aText);
}

#endif
