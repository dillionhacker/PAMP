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

#ifndef APACHE_MPM_SYMBIAN_H
#define APACHE_MPM_SYMBIAN_H

#include "apr.h"
#include "apr_pools.h"
#include "apr_thread_proc.h"
#include "apr_thread_cond.h"
#include "mpm.h"
#include "mpm_default.h"
#include "mpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mpm_state_t
{
    apr_thread_mutex_t* mutex;
    apr_thread_cond_t*  cond;
    int                 shutdown_pending;
    int                 restart_pending;
    int                 is_graceful;
    int                 n_workers;
    int                 n_active_workers;
} mpm_state_t;


typedef struct mpm_workers_t
{
    apr_thread_mutex_t* mutex;
    apr_thread_cond_t*  cond;
    int                 accept_available;
    int                 should_exit;
} mpm_workers_t;


typedef struct mpm_t
{
    mpm_state_t   state;
    mpm_workers_t workers;
} mpm_t;


typedef struct worker_args_t {
    int         slot;
    mpm_t*      mpm;
    apr_pool_t* pool;
} worker_args_t;



extern int mpm_thread_limit;
extern int mpm_min_spare_threads;
extern int mpm_max_spare_threads;
extern int mpm_max_requests_per_thread;


AP_DECLARE(void) ap_symbian_shutdown(int graceful);
AP_DECLARE(void) ap_symbian_restart(int graceful);


extern apr_status_t worker_start(apr_thread_t**    thread,
                                 apr_threadattr_t* thread_attr,
                                 int               slot,
                                 mpm_t*            mpm,
                                 apr_pool_t*       pool);
extern void         worker_main(worker_args_t* args);
extern void         worker_wakeup(void);

extern void         notify_about_worker(void);


#ifdef __cplusplus
}
#endif

#endif
