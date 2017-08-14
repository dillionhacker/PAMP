/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <e32std.h>
#include "ap_config.h"


namespace
{
    RMutex mutex;
    
    
    apr_status_t mpm_init(void)
        {
        apr_status_t
            rc = APR_SUCCESS;

        if (mutex.CreateLocal() != KErrNone)
            rc = APR_EGENERAL;

        return rc;
        }


    void mpm_finish(void)
        {
        mutex.Close();
        }
}


namespace
{
    void (*theCallback)(void*);
    void *theUserData;
}


extern "C"
{

void notify_about_worker()
{
    theCallback(theUserData);
}
    
}


extern "C"
{

AP_DECLARE(int) ap_symbian_mpm_init(void (*callback)(void*), void* userdata)
{
    theCallback = callback;
    theUserData = userdata;
    
    return mpm_init();
}

AP_DECLARE(void) ap_symbian_mpm_finish(void)
{
    mpm_finish();
}


void ap_symbian_mpm_lock(void)
    {
    mutex.Wait();
    }


void ap_symbian_mpm_unlock(void)
    {
    mutex.Signal();
    }
}
