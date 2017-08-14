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

#define CORE_PRIVATE

#include "httpd.h"
#include "http_log.h"
#include "ap_mpm.h"
#include "mpm_symbian.h"

#if !APR_HAS_THREADS
#error The Worker MPM requires APR threads, but they are unavailable.
#endif

/*
 * Global variables.
 */

server_rec*                                       ap_server_conf;
AP_DEFINE_EXPORTED_DATA(ap_generation_t volatile, ap_my_generation);

int mpm_min_spare_threads        = DEFAULT_MIN_SPARE_THREADS;
int mpm_max_spare_threads        = DEFAULT_MAX_SPARE_THREADS;
int mpm_thread_limit             = DEFAULT_THREAD_LIMIT;
int mpm_max_requests_per_thread  = DEFAULT_MAX_REQUESTS_PER_THREAD;


/*
 * Module variables.
 */ 

static int threads_to_start     = DEFAULT_THREADS_TO_START;

static int volatile mpm_state   = AP_MPMQ_STARTING;
static mpm_t*       the_mpm;


/*****************************************************************
 * 
 * GLOBAL FUNCTIONS
 *
 *****************************************************************/

AP_DECLARE(apr_status_t) ap_mpm_query(int query_code, int *result)
{
    switch(query_code){
        case AP_MPMQ_MAX_DAEMON_USED:
            *result = 1;
            return APR_SUCCESS;
        case AP_MPMQ_IS_THREADED:
            *result = AP_MPMQ_DYNAMIC;
            return APR_SUCCESS;
        case AP_MPMQ_IS_FORKED:
            *result = AP_MPMQ_NOT_SUPPORTED;
            return APR_SUCCESS;
        case AP_MPMQ_HARD_LIMIT_DAEMONS:
            *result = 1;
            return APR_SUCCESS;
        case AP_MPMQ_HARD_LIMIT_THREADS:
            *result = MAX_THREAD_LIMIT;
            return APR_SUCCESS;
        case AP_MPMQ_MAX_THREADS:
            *result = mpm_thread_limit;
            return APR_SUCCESS;
        case AP_MPMQ_MIN_SPARE_DAEMONS:
            *result = 0;
            return APR_SUCCESS;
        case AP_MPMQ_MIN_SPARE_THREADS:
            *result = mpm_min_spare_threads;
            return APR_SUCCESS;
        case AP_MPMQ_MAX_SPARE_DAEMONS:
            *result = 0;
            return APR_SUCCESS;
        case AP_MPMQ_MAX_SPARE_THREADS:
            *result = mpm_max_spare_threads;
            return APR_SUCCESS;
        case AP_MPMQ_MAX_REQUESTS_DAEMON:
            *result = mpm_max_requests_per_thread;
            return APR_SUCCESS;
        case AP_MPMQ_MAX_DAEMONS:
            *result = 1;
            return APR_SUCCESS;
        case AP_MPMQ_MPM_STATE:
            *result = mpm_state;
            return APR_SUCCESS;
    }
    return APR_ENOTIMPL;
}


AP_DECLARE(void) ap_symbian_shutdown(int graceful)
{
    ap_symbian_mpm_lock();

    /*
     * If the mpm mutex does not exist, then we are being shutdown 
     * before we have even started properly or after we have already 
     * started to shut down or restart. We'll just ignore it.
     */
    
    if (the_mpm) {
        apr_thread_mutex_lock(the_mpm->state.mutex);

        mpm_state = AP_MPMQ_STOPPING;

        if (!the_mpm->state.shutdown_pending) {
            the_mpm->state.shutdown_pending = 1;
            the_mpm->state.is_graceful = graceful;

            apr_thread_cond_signal(the_mpm->state.cond);
            apr_thread_mutex_unlock(the_mpm->state.mutex);
            
            apr_thread_mutex_lock(the_mpm->workers.mutex);
            the_mpm->workers.should_exit = 1;
            apr_thread_cond_broadcast(the_mpm->workers.cond);
            apr_thread_mutex_unlock(the_mpm->workers.mutex);
            
            worker_wakeup();
        }
        else
            apr_thread_mutex_unlock(the_mpm->state.mutex);
    }

    ap_symbian_mpm_unlock();
}


AP_DECLARE(void) ap_symbian_restart(int graceful)
{
    ap_symbian_mpm_lock();

    /*
     * If the mpm mutex does not exist, then we are being restarted 
     * before we have even started properly or after we have already 
     * started to shut down or restart. We'll just ignore it.
     */
    
    if (the_mpm) {
        apr_thread_mutex_lock(the_mpm->state.mutex);

        mpm_state = AP_MPMQ_STOPPING;
        
        if (!the_mpm->state.restart_pending) {
            the_mpm->state.restart_pending = 1;
            the_mpm->state.is_graceful = graceful;

            apr_thread_cond_signal(the_mpm->state.cond);
            apr_thread_mutex_unlock(the_mpm->state.mutex);
            
            apr_thread_mutex_lock(the_mpm->workers.mutex);
            the_mpm->workers.should_exit = 1;
            apr_thread_cond_broadcast(the_mpm->workers.cond);
            apr_thread_mutex_unlock(the_mpm->workers.mutex);

            worker_wakeup();
        }
        else
            apr_thread_mutex_unlock(the_mpm->state.mutex);
    }

    ap_symbian_mpm_unlock();
}


int ap_graceful_stop_signalled(void)
{
    int is_graceful = 0;

    ap_symbian_mpm_lock();

    if (the_mpm) {
        apr_thread_mutex_lock(the_mpm->state.mutex);
        is_graceful = the_mpm->state.is_graceful;
        apr_thread_mutex_unlock(the_mpm->state.mutex);
    }

    ap_symbian_mpm_unlock();

    return is_graceful;
}


/*****************************************************************
 * 
 * HELPERS
 *
 *****************************************************************/

static apr_status_t create_cond_mutex_pair(apr_thread_cond_t**  cond,
                                           apr_thread_mutex_t** mutex,
                                           apr_pool_t*          pool)
{
    apr_status_t
        rv = APR_SUCCESS;
    
    *cond = 0;
    *mutex = 0;
    
    rv = apr_thread_cond_create(cond, pool);
    
    if (rv == APR_SUCCESS) {
        rv = apr_thread_mutex_create(mutex, APR_THREAD_MUTEX_DEFAULT, pool);
        
        if (rv != APR_SUCCESS) {
            apr_thread_cond_destroy(*cond);
            *cond = 0;
        }
    }
    
    return rv;
}


static void destroy_cond_mutex_pair(apr_thread_cond_t*  cond,
                                    apr_thread_mutex_t* mutex)
{
    apr_thread_mutex_destroy(mutex);
    apr_thread_cond_destroy(cond);
}


static apr_status_t create_mpm(mpm_t* this, apr_pool_t* pool)
{
    apr_status_t
        rv = create_cond_mutex_pair(&this->state.cond, 
                                    &this->state.mutex, 
                                    pool);
    
    if (rv == APR_SUCCESS) {
        rv = create_cond_mutex_pair(&this->workers.cond, 
                                    &this->workers.mutex, 
                                    pool);
        
        if (rv == APR_SUCCESS) {
            this->state.shutdown_pending = 0;
            this->state.restart_pending = 0;
            this->state.is_graceful = 0;
            this->state.n_workers = 0;
            this->state.n_active_workers = 0;
            
            this->workers.accept_available = 1;
            this->workers.should_exit = 0;
        }
        else 
            destroy_cond_mutex_pair(this->state.cond, 
                                    this->state.mutex);
    }
    
    return rv;
}


static void destroy_mpm(mpm_t* this)
{
    destroy_cond_mutex_pair(this->workers.cond, this->workers.mutex);
    destroy_cond_mutex_pair(this->state.cond, this->state.mutex);
}


/*****************************************************************
 * 
 * MAIN
 *
 *****************************************************************/

static void join_threads(apr_thread_t **threads)
{
    int i;
    apr_status_t rv, thread_rv;

    for (i = 0; i < mpm_thread_limit; i++) {
        if (threads[i]) { /* if we ever created this thread */
            rv = apr_thread_join(&thread_rv, threads[i]);
            if (rv != APR_SUCCESS) {
                ap_log_error(APLOG_MARK, APLOG_ERR, rv, ap_server_conf,
                             "Failed to join worker thread %d.", i);
            }
        }
    }
}

static int start_threads(apr_thread_t**    threads,
                         apr_threadattr_t* thread_attr,
                         int               threads_to_start,
                         mpm_t*            mpm,
                         apr_pool_t*       pool)
{
    int i;
    int threads_started = 0;

    for (i = 0; threads_to_start && (i < mpm_thread_limit); ++i) {
        apr_status_t
            rv = worker_start(&threads[i], thread_attr, i, mpm, pool);
        
        if (rv == APR_SUCCESS)
            ++threads_started;
        else {
            threads[i] = 0;
            
            /* If thread creation fails, we assume that there
             * is some resource problem and that it's best to simply
             * bail out.
             */
            
            break;
        }

        --threads_to_start;
    }

    return threads_started;
}


static int worker_should_be_started(mpm_t* mpm)
{
    int
        start = 0;
    int
        n_spare = mpm->state.n_workers - mpm->state.n_active_workers;

    if ((n_spare < mpm_min_spare_threads) &&
        (mpm->state.n_workers < mpm_thread_limit))
        start = 1;

    return start;
}


static void server_main_loop(apr_thread_t**    threads,
                             apr_threadattr_t* thread_attr,
                             mpm_t*            mpm,
                             apr_pool_t*       pool)
{
    int
        do_exit = 0;

    while (!do_exit) {
        int
            start_worker = 0;
        
        apr_thread_mutex_lock(mpm->state.mutex);
    
        while (!mpm->state.restart_pending  && 
               !mpm->state.shutdown_pending &&
               !(start_worker = worker_should_be_started(mpm)))
            while (apr_thread_cond_wait(mpm->state.cond, 
                                        mpm->state.mutex) != APR_SUCCESS)
                ;

        if (mpm->state.restart_pending || mpm->state.shutdown_pending)
            do_exit = 1;
    
        apr_thread_mutex_unlock(mpm->state.mutex);

        if (!do_exit && start_worker) {
            int i;
            
            for (i = 0; i < mpm_thread_limit; ++i) {
                if (threads[i]) {
                    /* I'm a bit hesitant about this. The 'status' is
                     * modified by worker threads and read here and
                     * a lock is nowhere in sight. But that's the case
                     * in all other mpms as well.
                     */

                    /* This thread was started... */
                    
                    int
                        status = ap_scoreboard_image->servers[0][i].status;
                    
                    if ((status == SERVER_DEAD) || 
                        (status == SERVER_GRACEFUL)) {
                        /* ...and it either no longer exists or is about to 
                         * exit, so we can use this slot. But first it needs 
                         * to be joined.
                         */
                        apr_status_t
                            rv,
                            thread_rv;

                        /* There's a small leak here. Threads are created
                         * using the pchild pool from which the apr_thread_t
                         * instance is allocated. That memory will not be
                         * freed before pchild is destroyed, which only
                         * happens at shutdown or restart.
                         */

                        rv = apr_thread_join(&thread_rv, threads[i]);
                        
                        /* We zero it out in all cases. There's nothing
                         * we can do if it failed.
                         */
                        
                        threads[i] = 0;

                        if (rv != APR_SUCCESS) {
                            ap_log_error(APLOG_MARK, APLOG_ERR, 
                                         rv, ap_server_conf,
                                         "Failed to join worker thread %d.",
                                         i);
                        }
                        break;
                    }
                }
                else 
                    break;
            }

            if (i != mpm_thread_limit) {
                /*
                 * We found a slot.
                 */

                apr_status_t
                    rv;
                
                ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
                             "Too few spare threads. Starting a new "
                             "in slot %d.", i);

                rv = worker_start(&threads[i], thread_attr, i, mpm, pool);

                if (rv != APR_SUCCESS) {
                    ap_log_error(APLOG_MARK, APLOG_ERR, rv, ap_server_conf,
                                 "Failed to create worker thread.");
                }
            }
        }
    }
}


static void mpm_st_main(mpm_t* mpm, apr_pool_t* pchild)
{
    worker_args_t
        *args = malloc(sizeof(*args));

    if (args) {
        args->slot = 0;
        args->mpm  = mpm;
        args->pool = pchild;
        
        mpm_state = AP_MPMQ_RUNNING;

        worker_main(args); /* Ownership of args moved. */
    }
    else {
        ap_log_error(APLOG_MARK, APLOG_EMERG, errno, ap_server_conf,
                     "Could not allocated worker arguments. Terminating.");
        
        mpm->state.shutdown_pending = 1;
    }
}


static void mpm_mt_main(mpm_t* mpm, apr_pool_t* pchild)
{
    apr_thread_t 
        **threads;
    
    /* Allocate _cleared_ storage.
     */
    threads = calloc(1, sizeof(apr_thread_t*) * mpm_thread_limit);
    
    if (threads) {
        apr_threadattr_t 
            *thread_attr;
        apr_status_t
            rv = apr_threadattr_create(&thread_attr, pchild);
        
        if (rv == APR_SUCCESS) {
            int 
                n_threads = 0;
            
            apr_threadattr_detach_set(thread_attr, 0); /* Joinable. */
            
            if (ap_thread_stacksize != 0)
                apr_threadattr_stacksize_set(thread_attr, 
                                             ap_thread_stacksize);
            
            n_threads = start_threads(threads, thread_attr, 
                                      threads_to_start, mpm, pchild);
            
            if (n_threads != 0) {
                mpm_state = AP_MPMQ_RUNNING;
                
                ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
                             "Started %d(%d) initial workers.",
                             n_threads, threads_to_start);
                
                server_main_loop(threads, thread_attr, mpm, pchild);
                
                /* server_main_loop() returns when shutdown or
                 * restart has (for whatever reason) been initiated.
                 * Now all we need to do is to wait for the worker
                 * threads to finish.
                 */
                join_threads(threads);
            }
            else {
                ap_log_error(APLOG_MARK, APLOG_EMERG, 0, ap_server_conf,
                             "Could not create even a single worker "
                             "thread at startup. Terminating.");
                
                mpm->state.shutdown_pending = 1;
            }
        } else {
            ap_log_error(APLOG_MARK, APLOG_EMERG, rv, ap_server_conf,
                         "Could not create thread attributes. "
                         "Terminating.");
            
            mpm->state.shutdown_pending = 1;
        }
        
        free(threads);
    } else {
        ap_log_error(APLOG_MARK, APLOG_EMERG, errno, ap_server_conf,
                     "Could not allocated threads storage. Terminating.");
        
        mpm->state.shutdown_pending = 1;
    }
}


static void mpm_main(mpm_t* mpm, apr_pool_t* pconf)
{
    apr_status_t rv;
    
    apr_pool_t *pchild;

    rv = apr_pool_create(&pchild, pconf);

    if (rv == APR_SUCCESS) {
        apr_pool_tag(pchild, "pchild");
        
        /* This mpm is multi threaded but single process, so there should
         * be no need to run child_init? However, mod_proxy does its
         * initializations in child_init, so it must be called.
         */

        ap_run_child_init(pchild, ap_server_conf);
      
        if (mpm_thread_limit == 1) {
            ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
                         "Running in single-thread mode.");
                         
            mpm_st_main(mpm, pchild);
        }
        else {
            ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
                         "Running in multi-thread mode.");

            mpm_mt_main(mpm, pchild);
        }
        
        apr_pool_destroy(pchild);
    } else {
        ap_log_error(APLOG_MARK, APLOG_EMERG, rv, ap_server_conf,
                     "Could not create child pool. Terminating.");

        mpm->state.shutdown_pending = 1;
    }

    mpm_state = AP_MPMQ_STOPPING;
}


static void check_settings(server_rec* s)
{
    module
        *m = ap_find_linked_module("mod_php5.c");

    if (m) {
        /* mod_php5 is loaded
         */
        if (mpm_thread_limit > 1) {
            /* and multithreading is specified but they do not support 
             * threading yet. Consequently, we revert back to single-thread 
             * mode.
             */
            
            ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, s,
                         "ThreadLimit %d > 1 and mod_php5 "
                         "loaded, but it does not (yet) support threading. "
                         "ThreadLimit reset to 1.",
                         mpm_thread_limit);

            mpm_thread_limit = 1;
            threads_to_start = mpm_thread_limit;
            mpm_max_spare_threads = mpm_thread_limit - 1;
            mpm_min_spare_threads = mpm_max_spare_threads;
        }
    }
    
    if (threads_to_start > mpm_thread_limit) {
        ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, s,
                     "StartThreads of %d exceeds ThreadLimit of %d, "
                     "reset to %d.",
                     threads_to_start,
                     mpm_thread_limit,
                     mpm_thread_limit);

        threads_to_start = mpm_thread_limit;
    }

    if (mpm_max_spare_threads >= mpm_thread_limit) {
        ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, s,
                     "MaxSpareThreads of %d exceeds or is equal "
                     "to ThreadLimit of %d, reset to %d.",
                     mpm_max_spare_threads,
                     mpm_thread_limit,
                     mpm_thread_limit - 1);

        mpm_max_spare_threads = mpm_thread_limit - 1;
    }

    if (mpm_min_spare_threads > mpm_max_spare_threads) {
        ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, s,
                     "MinSpareThreads of %d exceeds "
                     "MaxSpareThreads of %d, reset to %d.",
                     mpm_min_spare_threads,
                     mpm_max_spare_threads,
                     mpm_max_spare_threads);

        mpm_min_spare_threads = mpm_max_spare_threads;
    }

    if (ap_thread_stacksize == 0)
        ap_thread_stacksize = DEFAULT_THREAD_STACKSIZE;
}


int ap_mpm_run(apr_pool_t *pconf, apr_pool_t *plog, server_rec *s)
{
    static int is_graceful;
    
    apr_status_t rv;
    mpm_t mpm;

    ap_symbian_mpm_lock();
    rv = create_mpm(&mpm, pconf);
    
    if (rv == APR_SUCCESS)
        the_mpm = &mpm;
    
    ap_symbian_mpm_unlock();

    if (rv == APR_SUCCESS) {
        check_settings(s);
        
        if (!is_graceful) {
            rv = ap_run_pre_mpm(s->process->pool, SB_SHARED);
            
            if (rv == OK) { /* Same as APR_SUCCESS */
                /* Reset the generation number in the global score as we 
                 * just got a new cleared scoreboard.
                 */
                ap_scoreboard_image->global->running_generation = 
                    ap_my_generation;
            }
            else
                rv = 1;
        }
        
        if (rv == APR_SUCCESS) {
            mpm_main(&mpm, pconf);

            if (mpm.state.shutdown_pending) {
                ap_log_error(APLOG_MARK, APLOG_INFO, 0,
                             ap_server_conf, "Shutting down.");
        
                rv = 1;
            }
            else {
                /* Advance to the next generation. 
                 */
                ++ap_my_generation;
                ap_scoreboard_image->global->running_generation = 
                    ap_my_generation;
        
                ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
                             "Attempting to restart.");
                
                rv = 0;
            }

            is_graceful = mpm.state.is_graceful;
        } else {
            ap_log_error(APLOG_MARK, APLOG_EMERG, 0, s,
                         "Could not run pre_mpms. Terminating.");
            
            rv = 1;
        }

        ap_symbian_mpm_lock();
        the_mpm = 0;
        
        destroy_mpm(&mpm);
        ap_symbian_mpm_unlock();

    } else {
        ap_log_error(APLOG_MARK, APLOG_EMERG, rv, s,
                     "Could not create synchronization objects (mutex, cond)");
        
        rv = 1;
    }

    return rv;
}


/*****************************************************************
 * 
 * HOOKS
 *
 *****************************************************************/

/* This really should be a post_config hook, but the error log is already
 * redirected by that point, so we need to do this in the open_logs phase.
 */
static int mpm_open_logs(apr_pool_t *pconf, 
                         apr_pool_t *plog, 
                         apr_pool_t *ptemp, 
                         server_rec *s)
{
    ap_server_conf = s;

    if (ap_setup_listeners(ap_server_conf) < 1) {
        ap_log_error(APLOG_MARK, APLOG_ALERT|APLOG_STARTUP, 0,
                     NULL, "No listening sockets available, shutting down.");
        
        return DONE;
    }

    return OK;
}


static int mpm_pre_config(apr_pool_t *pconf, 
                          apr_pool_t *plog,
                          apr_pool_t *ptemp)
{
    mpm_state = AP_MPMQ_STARTING;

    ap_listen_pre_config();

    threads_to_start           = DEFAULT_THREADS_TO_START;

    mpm_min_spare_threads       = DEFAULT_MIN_SPARE_THREADS;
    mpm_max_spare_threads       = DEFAULT_MAX_SPARE_THREADS;
    mpm_thread_limit            = DEFAULT_THREAD_LIMIT;
    mpm_max_requests_per_thread = DEFAULT_MAX_REQUESTS_PER_THREAD;

    ap_thread_stacksize        = DEFAULT_THREAD_STACKSIZE;
    ap_max_mem_free            = DEFAULT_MAX_MEM_FREE;

    ap_extended_status = 0;

    return OK;
}

static void mpm_hooks(apr_pool_t *p)
{
    /* The worker open_logs phase must run before the core's, or stderr
     * will be redirected to a file, and the messages won't print to the
     * console.
     */
    static const char *const aszSucc[] = {"core.c", NULL};

    ap_hook_open_logs(mpm_open_logs, NULL, aszSucc, APR_HOOK_MIDDLE);
    /* we need to set the MPM state before other pre-config hooks use MPM query
     * to retrieve it, so register as REALLY_FIRST
     */
    ap_hook_pre_config(mpm_pre_config, NULL, NULL, APR_HOOK_REALLY_FIRST);
}


/*****************************************************************
 * 
 * COMMAND
 *
 *****************************************************************/

static const char *set_threads_to_start(cmd_parms*  cmd, 
                                        void*       dummy, 
                                        const char* arg)
{
    const char *err = ap_check_cmd_context(cmd, GLOBAL_ONLY);
    
    if (err != NULL) {
        return err;
    }

    threads_to_start = atoi(arg);
    
    if (threads_to_start > MAX_THREAD_LIMIT) {
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL,
                     "StartThreads of %d exceeds compile time "
                     "limit of %d, lowering StartThreads to %d.",
                     threads_to_start,
                     MAX_THREAD_LIMIT,
                     MAX_THREAD_LIMIT);
        
        threads_to_start = MAX_THREAD_LIMIT;
    }
    else if (threads_to_start < 1) {
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL,
                     "StartThreads set to a value (%d) "
                     "less than 1, reset to 1.",
                     threads_to_start);
        
        threads_to_start = 1;
    }

    return NULL;
}


static const char *set_min_spare_threads(cmd_parms*  cmd, 
                                         void*       dummy, 
                                         const char* arg)
{
    const char *err = ap_check_cmd_context(cmd, GLOBAL_ONLY);
    
    if (err != NULL) {
        return err;
    }

    mpm_min_spare_threads = atoi(arg);
    
    if (mpm_min_spare_threads > MAX_THREAD_LIMIT) {
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL,
                     "MinSpareThreads of %d exceeds compile time "
                     "limit of %d, lowering MinSpareThreads to %d.",
                     mpm_min_spare_threads,
                     MAX_THREAD_LIMIT - 1,
                     MAX_THREAD_LIMIT - 1);
        
        mpm_min_spare_threads = MAX_THREAD_LIMIT - 1;
    }
    else if (mpm_min_spare_threads < 1) {
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL,
                     "MinSpareThreads set to a value (%d) "
                     "less than 1, reset to 1.",
                     mpm_min_spare_threads);
        
        mpm_min_spare_threads = 1;
    }

    return NULL;
}


static const char *set_max_spare_threads(cmd_parms*  cmd, 
                                         void*       dummy, 
                                         const char* arg)
{
    const char *err = ap_check_cmd_context(cmd, GLOBAL_ONLY);
    
    if (err != NULL) {
        return err;
    }

    mpm_max_spare_threads = atoi(arg);
    
    if (mpm_max_spare_threads > MAX_THREAD_LIMIT) {
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL,
                     "MaxSpareThreads of %d exceeds compile time "
                     "limit of %d, lowering MaxSpareThreads to %d.",
                     mpm_max_spare_threads,
                     MAX_THREAD_LIMIT - 1,
                     MAX_THREAD_LIMIT - 1);
        
        mpm_max_spare_threads = MAX_THREAD_LIMIT - 1;
    }
    else if (mpm_max_spare_threads < 1) {
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL,
                     "MaxSpareThreads set to a value (%d) "
                     "less than 1, reset to 1.", 
                     mpm_max_spare_threads);
        
        mpm_max_spare_threads = 1;
    }

    return NULL;
}


static const char *set_thread_limit(cmd_parms*  cmd, 
                                    void*       dummy, 
                                    const char* arg)
{
    const char *err = ap_check_cmd_context(cmd, GLOBAL_ONLY);
    
    if (err != NULL) {
        return err;
    }

    mpm_thread_limit = atoi(arg);
    
    if (mpm_thread_limit > MAX_THREAD_LIMIT) {
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL,
                     "ThreadLimit of %d exceeds compile time limit "
                     "of %d, lowering ThreadLimit to %d.",
                     mpm_thread_limit, 
                     MAX_THREAD_LIMIT,
                     MAX_THREAD_LIMIT);
       
        mpm_thread_limit = MAX_THREAD_LIMIT;
    }
    else if (mpm_thread_limit < 1) {
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL,
                     "ThreadLimit set to a value (%d) "
                     "less than 1, reset to %d.",
                     mpm_thread_limit,
                     DEFAULT_THREAD_LIMIT);
        
        mpm_thread_limit = DEFAULT_THREAD_LIMIT;
    }
    
    return NULL;
}


static const char *set_max_requests_per_thread(cmd_parms*  cmd, 
                                               void*       dummy, 
                                               const char* arg)
{
    const char *err = ap_check_cmd_context(cmd, GLOBAL_ONLY);
    
    if (err != NULL) {
        return err;
    }

    mpm_max_requests_per_thread = atoi(arg);
    
    if (mpm_max_requests_per_thread < 0) {
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL,
                     "MaxRequestsPerThread was set below 0"
                     "reset to 0, but this may not be what you want.");
        
        mpm_max_requests_per_thread = 0;
    }

    return NULL;
}


static const command_rec mpm_cmds[] = {
LISTEN_COMMANDS,
AP_INIT_TAKE1( "StartThreads", set_threads_to_start, NULL, RSRC_CONF,
  "Number of threads to launch at server startup"),
AP_INIT_TAKE1( "MinSpareThreads", set_min_spare_threads, NULL, RSRC_CONF,
  "Minimum number of idle children, to handle request spikes"),
AP_INIT_TAKE1( "MaxSpareThreads", set_max_spare_threads, NULL, RSRC_CONF,
  "Maximum number of idle children" ),
AP_INIT_TAKE1( "ThreadLimit", set_thread_limit, NULL, RSRC_CONF,
  "Maximum number of threads alive at the same time (concurrent requests)" ),
AP_INIT_TAKE1( "MaxRequestsPerThread", set_max_requests_per_thread, NULL, 
               RSRC_CONF, "Maximum number of requests served by a thread" ),
{ NULL }
};


/*****************************************************************
 * 
 * MODULE
 *
 *****************************************************************/

module AP_MODULE_DECLARE_DATA mpm_symbian_module = {
    MPM20_MODULE_STUFF,
    NULL,                    /* hook to run before apache parses args */
    NULL,                    /* create per-directory config structure */
    NULL,                    /* merge per-directory config structures */
    NULL,                    /* create per-server config structure */
    NULL,                    /* merge per-server config structures */
    mpm_cmds,                /* command apr_table_t */
    mpm_hooks                /* register_hooks */
};

