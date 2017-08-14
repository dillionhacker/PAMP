/* Copyright 2001-2004 The Apache Software Foundation
 * Partly Copyright 2006 Nokia Corporation. All rights reserved
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

#ifndef APACHE_MPM_DEFAULT_H
#define APACHE_MPM_DEFAULT_H

#ifdef __cplusplus
extern "C" {
#endif

/* The default maximum number of threads. That is, the maximum number
 * of concurrently handled requests.
 *
 * Corresponding directive: ThreadLimit
 */
#ifndef DEFAULT_THREAD_LIMIT
#define DEFAULT_THREAD_LIMIT 4
#endif

/* The ThreadLimit directive can be used to override DEFAULT_THREAD_LIMIT,
 * but cannot be set higher than MAX_THREAD_LIMIT. This is a sort of 
 * compile-time limit to help catch typos.
 */
#ifndef MAX_THREAD_LIMIT
#define MAX_THREAD_LIMIT 16
#endif

/* The default number of threads to start at startup.
 *
 * Corresponding directive: ThreadsToStart
 */
#ifndef DEFAULT_THREADS_TO_START
#define DEFAULT_THREADS_TO_START  1
#endif

/* The default minimum amount of spare threads kept available 
 * for sudden request spikes.
 *
 * Corresponding directive: MinSpareThreads
 */
#ifndef DEFAULT_MIN_SPARE_THREADS
#define DEFAULT_MIN_SPARE_THREADS 1
#endif

/* The default maximum amount of spare threads kept available 
 * for sudden request spikes.
 *
 * Corresponding directive: MaxSpareThreads
 */
#ifndef DEFAULT_MAX_SPARE_THREADS
#define DEFAULT_MAX_SPARE_THREADS 1
#endif

/* Number of requests to handle in one thread.  If <= 0,
 * the threads don't die off.
 *
 * Corresponding directive: MaxRequestsPerThread
 */
#ifndef DEFAULT_MAX_REQUESTS_PER_THREAD
#define DEFAULT_MAX_REQUESTS_PER_THREAD 0
#endif

/* The default thread stacksize.
 *
 * Corresponding directive: ThreadStackSize  
 */
#ifndef DEFAULT_THREAD_STACKSIZE
#define DEFAULT_THREAD_STACKSIZE 0x10000
#endif

/* The maximum amount of unused memory an allocator, used for request
 * handling, may hold, before starting to actually free memory.
 *
 * Corresponding directive: MaxMemFree
 */
#ifndef DEFAULT_MAX_MEM_FREE
#define DEFAULT_MAX_MEM_FREE 0
#endif

#ifdef __cplusplus
}
#endif

#endif /* AP_MPM_DEFAULT_H */
