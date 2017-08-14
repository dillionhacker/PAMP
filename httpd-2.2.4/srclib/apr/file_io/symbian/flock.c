/* Copyright 2007 Nokia Corporation
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

#include "apr_arch_file_io.h"


APR_DECLARE(apr_status_t) apr_file_lock(apr_file_t *thefile, int type)
{
    /* FIXME: OpenC has fcntl, but F_SETLK, F_SETLKW and F_GETLK are
     * FIXME: not supported. For the time being we just pretend locking
     * FIXME: always succeeds.
     *
     * FIXME: REVISIT WHEN THREADS ARE INTRODUCED!
     */
    
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_file_unlock(apr_file_t *thefile)
{
    /* FIXME: OpenC has fcntl, but F_SETLK, F_SETLKW and F_GETLK are
     * FIXME: not supported. For the time being we just pretend 
     * FIXME: unlocking always succeeds.
     *
     * FIXME: REVISIT WHEN THREADS ARE INTRODUCED!
     */

    return APR_SUCCESS;
}
