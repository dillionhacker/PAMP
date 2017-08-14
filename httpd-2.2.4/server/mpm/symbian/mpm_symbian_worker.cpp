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

extern "C" {
#include "httpd.h"
#include "http_log.h"
#include "http_connection.h"
#include <unistd.h>
#include <arpa/inet.h>
#include "mpm_symbian.h"
}
#include <e32base.h>


/*****************************************************************
 * 
 * STATIC
 *
 *****************************************************************/

static apr_status_t get_listener(ap_listen_rec** lr,
                                 int*            last_poll_idx,
                                 apr_pollset_t*  pollset)
{
    apr_status_t
        rv = APR_SUCCESS;

    if (!pollset) {
        /* We have just one listener, so we can just grab it.
         */

        *lr = ap_listeners;
    } else {
        apr_int32_t 
            numdesc = 0;
        const apr_pollfd_t 
            *pdesc = NULL;
            
        rv = apr_pollset_poll(pollset, -1, &numdesc, &pdesc);
        
        if (rv == APR_SUCCESS) {
            /* We can always use pdesc[0], but sockets at position N
             * could end up completely starved of attention in a very
             * busy server. Therefore, we round-robin across the
             * returned set of descriptors. While it is possible that
             * the returned set of descriptors might flip around and
             * continue to starve some sockets, we happen to know the
             * internal pollset implementation retains ordering
             * stability of the sockets. Thus, the round-robin should
             * ensure that a socket will eventually be serviced.
             */
            if (*last_poll_idx >= numdesc)
                *last_poll_idx = 0;
            
            /* Grab a listener record from the client_data of the poll
             * descriptor, and advance our saved index to round-robin
             * the next fetch.
             *
             * ### hmm... this descriptor might have POLLERR rather
             * ### than POLLIN
             */

            void
                *data = pdesc[*last_poll_idx++].client_data;
            
            *lr = static_cast<ap_listen_rec*>(data);
        }
        else {
            ap_log_error(APLOG_MARK, APLOG_ERR, rv,
                         ap_server_conf, "apr_pollset_poll: (listen)");
        }
    }
    
    return rv;
}


static int adjust_workers(int    slot, 
                          mpm_t* mpm,
                          int*   n_spare)
{
    /* This must be called with mpm.state.mutex locked. */
    
    int
        do_exit = 0;

    *n_spare = mpm->state.n_workers - mpm->state.n_active_workers;
    
    if (*n_spare > mpm_max_spare_threads) {
        /*
         * More spare threads than needed. Time to exit.
         */
        
        do_exit = 1;
    }
    else if (*n_spare < mpm_min_spare_threads) {
        /*
         * Too few threads. Time to wakeup the main thread
         * and let it decide whether another one should
         * be created, but only if the max number of threads
         * has not been reached.
         */
        
        if (mpm->state.n_workers < mpm_thread_limit)
            apr_thread_cond_signal(mpm->state.cond);
    }

    return do_exit;
}


static int count_listeners()
{
    int
        count = 0;
    ap_listen_rec 
        *lr;
    
    for (lr = ap_listeners; lr; lr = lr->next)
        count++;
    
    return count;
}


static apr_status_t create_pollset(apr_pollset_t** pollset, 
                                   apr_pool_t*     pool)
{
    apr_status_t
        rv = APR_SUCCESS;
    
    *pollset = 0;
    
    if (ap_listeners->next) {
        /* There's more than one listener. We really have to use 
         * a pollset.
         */
        ap_listen_rec *lr = NULL;
                
        rv = apr_pollset_create(pollset, count_listeners(), pool, 0);
        
        if (rv == APR_SUCCESS) {
            for (lr = ap_listeners; lr != NULL; lr = lr->next) {
                apr_pollfd_t pfd = {0};
                
                pfd.desc_type = APR_POLL_SOCKET;
                pfd.desc.s = lr->sd;
                pfd.reqevents = APR_POLLIN;
                pfd.client_data = lr;
                
                /* Will not fail; the pollset contains room for
                 * all descriptors.
                 */
                (void) apr_pollset_add(*pollset, &pfd);
            }
        }
    }

    return rv;
}


static void worker_loop(int                 slot,
                        mpm_t*              mpm,
                        apr_pool_t*         ptrans,
                        ap_sb_handle_t*     sbh,
                        apr_bucket_alloc_t* bucket_alloc,
                        apr_pollset_t*      pollset)
{
    apr_status_t rv = APR_SUCCESS;
    ap_listen_rec *lr = NULL;
    int last_poll_idx = 0;
    int do_exit = 0;
    int requests_per_thread = 0;
    
    while (!do_exit) {
        /* (Re)initialize this child to a pre-connection state. */
        apr_pool_clear(ptrans);

        if ((mpm_max_requests_per_thread > 0
             && requests_per_thread++ >= mpm_max_requests_per_thread)) {
                ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
                             "worker(%d): %d requests handled. Exiting.",
                             slot, requests_per_thread - 1);

                do_exit = 1;
                continue;
            }
        
        (void) ap_update_child_status(sbh, SERVER_READY, NULL);
        
        apr_thread_mutex_lock(mpm->workers.mutex);

        while (!mpm->workers.should_exit && !mpm->workers.accept_available)
            while (apr_thread_cond_wait(mpm->workers.cond, mpm->workers.mutex))
                ;
     
        if (mpm->workers.should_exit) {
            apr_thread_cond_signal(mpm->workers.cond);
            apr_thread_mutex_unlock(mpm->workers.mutex);

            do_exit = 1;
            continue;
        }
        else {
            mpm->workers.accept_available = 0;
            apr_thread_mutex_unlock(mpm->workers.mutex);

            rv = get_listener(&lr, &last_poll_idx, pollset);
            
            if (rv == APR_SUCCESS) {
                conn_rec *current_conn;
                void *csd;
                int n_spare;
                
                ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf,
                             "worker(%d): Waiting for request.", slot);
                
                rv = lr->accept_func(&csd, lr, ptrans);

                ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf,
                             "worker(%d): Accept returned.", slot);

                if (rv != APR_SUCCESS) {
                    ap_log_error(APLOG_MARK, APLOG_ERR, rv, ap_server_conf,
                                 "worker(%d): Accepting socket failed, "
                                 "shutting down.", slot);
                    
                    ap_symbian_shutdown(0);

                    do_exit = 1;
                    continue;
                }
                
                if (mpm->workers.should_exit) {
                    do_exit = 1;
                    continue;
                }

                apr_thread_mutex_lock(mpm->state.mutex);
                ++mpm->state.n_active_workers;
                adjust_workers(slot, mpm, &n_spare);
                apr_thread_mutex_unlock(mpm->state.mutex);

                apr_thread_mutex_lock(mpm->workers.mutex);
                mpm->workers.accept_available = 1;
                apr_thread_cond_signal(mpm->workers.cond);
                apr_thread_mutex_unlock(mpm->workers.mutex);

                apr_socket_t
                    *socket = static_cast<apr_socket_t*>(csd);
                
                current_conn = ap_run_create_connection(ptrans, 
                                                        ap_server_conf, 
                                                        socket,
                                                        slot, sbh, 
                                                        bucket_alloc);
                
                if (current_conn) {
                    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf,
                                 "worker(%d): Processing connection.", slot);

                    ap_process_connection(current_conn, csd);

                    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf,
                                 "worker(%d): Connection processed.", slot);

                    ap_lingering_close(current_conn);

                    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf,
                                 "worker(%d): Connection closed.", slot);
                }
                else 
                    ap_log_error(APLOG_MARK, APLOG_WARNING, 0, ap_server_conf,
                                 "worker(%d): ap_run_create_connection "
                                 "failed.", slot);

                apr_thread_mutex_lock(mpm->state.mutex);
                do_exit = adjust_workers(slot, mpm, &n_spare);
                --mpm->state.n_active_workers;
                apr_thread_mutex_unlock(mpm->state.mutex);

                if (do_exit)
                    ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
                                 "worker(%d): Too many (%d/%d) spare threads. "
                                 "Exiting.", 
                                 slot, n_spare, mpm_max_spare_threads);
            } else {
                apr_thread_mutex_lock(mpm->workers.mutex);
                mpm->workers.accept_available = 1;
                apr_thread_cond_signal(mpm->workers.cond);
                apr_thread_mutex_unlock(mpm->workers.mutex);
                
                do_exit = 1;
            }
        }
    }
}


static void worker_main(void* dummy)
{
    CActiveScheduler
        *pCurrent   = CActiveScheduler::Current(), // Should always be 0.
        *pScheduler = 0;
    
    if (!pCurrent) {
        pScheduler = new (ELeave) CActiveScheduler;
            
        CActiveScheduler::Install(pScheduler);
    }
        
    worker_main(static_cast<worker_args_t*>(dummy));
    
    if (pScheduler) {
        CActiveScheduler::Install(0);
        delete pScheduler;
    }

}


static void* APR_THREAD_FUNC worker_thread(apr_thread_t* thd, void* dummy)
{
    TInt
        rc = KErrNone;
    
    CTrapCleanup
        *pCleanupStack = CTrapCleanup::New();

    notify_about_worker();

    if (pCleanupStack) {
        TRAP(rc, worker_main(dummy));
        
        if (rc != KErrNone) {
            ap_log_error(APLOG_MARK, APLOG_EMERG, 0, ap_server_conf,
                         "Worker %d exited due to Symbian leave. "
                         "Terminating.",
                         static_cast<worker_args_t*>(dummy)->slot);

            ap_symbian_shutdown(false);
        }
    } 
    else {
        ap_log_error(APLOG_MARK, APLOG_EMERG, 0, ap_server_conf,
                     "Could not create cleanup stack for worker %d. "
                     "Terminating.",
                     static_cast<worker_args_t*>(dummy)->slot);
        
        ap_symbian_shutdown(false);
    }

    /* Thread deaths are noticed via the use of OS mechanisms,
     * no need to notify.
     */
    apr_thread_exit(thd, 0);
    
    return 0;
}


/*****************************************************************
 * 
 * EXTERN
 *
 *****************************************************************/

namespace
{
    _LIT(KFormat, "Worker (%d, %d, %d)");
    
    enum {
        KFormatSize = 13 + 3 * 10
    };

    void set_thread_name(TInt pid, TInt tid, TInt slot)
    {
        TBuf<KFormatSize>
            name;
        
        name.Format(KFormat, pid, tid, slot);

        RThread::RenameMe(name);
    }


    void set_thread_name(TInt slot)
    {
        set_thread_name(getpid(), pthread_self(), slot);
    }
}

void worker_main(worker_args_t* args)
{
    apr_status_t rv;

    int slot = args->slot;
    mpm_t* mpm = args->mpm;
    apr_pool_t* pool = args->pool;

    apr_allocator_t *allocator;

    ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
                 "worker(%d): Starting.", slot);

    free(args);

    set_thread_name(slot);

    ap_scoreboard_image->servers[0][slot].pid = getpid();
    ap_scoreboard_image->servers[0][slot].generation = ap_my_generation;
    ap_update_child_status_from_indexes(0, slot, SERVER_STARTING, NULL);

    rv = apr_allocator_create(&allocator);
    
    if (rv == APR_SUCCESS) {
        apr_pool_t 
            *pworker;

        apr_allocator_max_free_set(allocator, ap_max_mem_free);
        
        rv = apr_pool_create_ex(&pworker, pool, NULL, allocator);

        if (rv == APR_SUCCESS) {
            apr_pollset_t
                *pollset;
            
            apr_allocator_owner_set(allocator, pworker);

            rv = create_pollset(&pollset, pworker);

            if (rv == APR_SUCCESS) {
                apr_pool_t
                    *ptrans;
                
                rv = apr_pool_create(&ptrans, pworker);

                if (rv == APR_SUCCESS) {
                    ap_sb_handle_t 
                        *sbh;
                    apr_bucket_alloc_t 
                        *bucket_alloc;

                    apr_pool_tag(ptrans, "transaction");
                    
                    ap_create_sb_handle(&sbh, pworker, 0, slot);
                    (void) ap_update_child_status(sbh, SERVER_READY, 
                                                  (request_rec *) NULL);
                    
                    /* If the allocation fails there'll be a crash.
                     * I.e. if the function returns, everything is ok.
                     */
                    bucket_alloc = apr_bucket_alloc_create(pworker);
                    
                    worker_loop(slot, mpm, ptrans, 
                                sbh, bucket_alloc, pollset);
                    
                    apr_pool_destroy(ptrans);
                }
                else
                    ap_log_error(APLOG_MARK, APLOG_ERR, rv, ap_server_conf,
                                 "worker(%d): Failed to create worker "
                                 "transaction pool.", slot);
            }
            else
                ap_log_error(APLOG_MARK, APLOG_ERR, rv, ap_server_conf,
                             "worker(%d): Failed to create pollset.", slot);
            
            apr_pool_destroy(pworker);
        }
        else {
            ap_log_error(APLOG_MARK, APLOG_ERR, rv, ap_server_conf,
                         "worker(%d): Failed to create worker pool.", slot);
            
            apr_allocator_destroy(allocator);
        }
    }
    else
        ap_log_error(APLOG_MARK, APLOG_ERR, rv, ap_server_conf, 
                     "worker(%d): Failed to the create allocator.", slot);
        
    (void) ap_update_child_status_from_indexes(0, slot, SERVER_DEAD,
                                               (request_rec*)NULL);
    ap_scoreboard_image->servers[0][slot].tid = 0;

    apr_thread_mutex_lock(mpm->state.mutex);
    --mpm->state.n_workers;
    /*
     * Wakeup main thread in case a new worker must be created.
     */
    apr_thread_cond_signal(mpm->state.cond);
    apr_thread_mutex_unlock(mpm->state.mutex);

    ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
                 "worker(%d): Exiting.", slot);
}


apr_status_t worker_start(apr_thread_t**    thread,
                          apr_threadattr_t* thread_attr,
                          int               slot,
                          mpm_t*            mpm,
                          apr_pool_t*       pool)
{
    apr_status_t
        rv;
    worker_args_t
        *args;

    args = (worker_args_t*) malloc(sizeof(*args));

    if (args) {
        args->slot = slot;
        args->mpm  = mpm;
        args->pool = pool;
        
        ap_update_child_status_from_indexes(0, slot, SERVER_STARTING, NULL);

        apr_thread_mutex_lock(mpm->state.mutex);
        ++mpm->state.n_workers;

        rv = apr_thread_create(thread, thread_attr, worker_thread, args, pool);

        if (rv != APR_SUCCESS) {
            --mpm->state.n_workers;
            
            ap_update_child_status_from_indexes(0, slot, SERVER_DEAD, NULL);

            *thread = 0;
            
            free(args);
        }
        
        apr_thread_mutex_unlock(mpm->state.mutex);
    }
    else
        rv = APR_ENOMEM;

    return rv;
}


void worker_wakeup(void)
{
    /*
     * No APR? Simpler and faster this way.
     */
    
    int sd;
    int warn = 0;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    if (sd >= 0) {
        struct sockaddr_in in;
        
        memset(&in, 0, sizeof(in));
        in.sin_family = AF_INET;
        in.sin_addr.s_addr = inet_addr("127.0.0.1");
        in.sin_port = htons(ap_listeners->bind_addr->port);

        if (connect(sd, (const struct sockaddr*) &in, sizeof(in)) >= 0) {
            ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
                         "Waked up accepting thread.");
            
        }
        else {
            warn = errno;
            
            ap_log_error(APLOG_MARK, APLOG_ERR, errno, ap_server_conf,
                         "Failed to connect for wakeup.");
        }

        close(sd);
            
    } else {
        warn = errno;
        
        ap_log_error(APLOG_MARK, APLOG_ERR, errno, ap_server_conf,
                     "Failed to create socket for wakeup.");
    }

    if (warn)
        ap_log_error(APLOG_MARK, APLOG_ERR, warn, ap_server_conf,
                     "Worker thread will probably not exit.");
}


apr_status_t symbian_accept(void          **accepted, 
                            ap_listen_rec *lr,
                            apr_pool_t    *ptrans)
{
    apr_socket_t *csd;
    apr_status_t status;

    *accepted = NULL;
    status = apr_socket_accept(&csd, lr->sd, ptrans);

    /*
     * The approach on other platforms is to consider all errors but a 
     * selected few as fatal errors. On Symbian we consider all errors
     * as fatal (at least for now) until there is evidence that some
     * errors actually are not fatal.
     */

    if (status == APR_SUCCESS)
        *accepted = csd;

    return status;
}
