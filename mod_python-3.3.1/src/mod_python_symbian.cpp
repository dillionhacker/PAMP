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


#include <cspyinterpreter.h>
#include <apr_pools.h>
#include <ap_config.h>
#include <httpd.h>
#include <http_log.h>
#include <pthread.h>
#include <Python.h>


namespace
{
    const char* POOL_KEY = "mod_python:pys60";

    const void* MAIN_THREAD_VALUE = (void*) 1;

    pthread_key_t is_initialized;


    CSPyInterpreter* pTheInterpreter;
}


namespace
{
    
apr_status_t cleanup_process(CSPyInterpreter* pInterpreter)
{
    /* Safe, we are back in single-thread mode.
     */
    pTheInterpreter = 0;
    
    delete pInterpreter;

    pthread_setspecific(is_initialized, 0);
    pthread_key_delete(is_initialized);
    
    return APR_SUCCESS;
}


apr_status_t cleanup_process(void* pData)
{
    return cleanup_process(static_cast<CSPyInterpreter*>(pData));
}


void cleanup_thread(CSPyInterpreter* pInterpreter)
{
    pInterpreter->FinalizeForeignThread();
}


void cleanup_thread(void* pData)
{
    cleanup_thread(static_cast<CSPyInterpreter*>(pData));
}

}


extern "C"
{

apr_status_t PyS60_Initialize(apr_pool_t* pPool)
{
    apr_status_t
        rv = APR_SUCCESS;
    void
        *pData;

    apr_pool_userdata_get(&pData, POOL_KEY, pPool);

    if (pData == 0) {
        CSPyInterpreter
            *pInterpreter = 0;
        
        TRAP_IGNORE(pInterpreter = CSPyInterpreter::NewInterpreterL());

        if (pInterpreter) {
            rv = apr_pool_userdata_set(pInterpreter, 
                                       POOL_KEY, cleanup_process, pPool);
        
            if (rv == APR_SUCCESS) {
                /* Safe, we are in single-thread mode.
                 */
                pTheInterpreter = pInterpreter;
                
                if (pthread_key_create(&is_initialized, cleanup_thread) == 0) {
                    /* cleanup_thread() will never be called for this data, 
                     * as the key will be destroyed in cleanup_process(), 
                     * which is called when the pool is destroyed and that 
                     * happens before thread destruction.
                     */
                    if (pthread_setspecific(is_initialized, 
                                            MAIN_THREAD_VALUE) != 0) {
                        ap_log_perror(APLOG_MARK, APLOG_EMERG, 0, 0, 
                                      "Could not set thread-specific data.");
                        
                        rv = APR_EGENERAL;
                    }
                }
                else {
                    ap_log_perror(APLOG_MARK, APLOG_EMERG, 0, 0, 
                                  "Could not create pthread-key.");

                    rv = APR_EGENERAL;
                }
            }
            else {
                ap_log_perror(APLOG_MARK, APLOG_EMERG, rv, pPool, 
                              "Could not store Python interpreter "
                              "in pool userdata.");
            }
        }
        else {
            ap_log_perror(APLOG_MARK, APLOG_EMERG, 0, pPool,
                          "Could not create Python interpreter instance.");
            
            rv = APR_EGENERAL;
        }
    }
    else {
        /* If something is found, then this function has been called twice,
         * although it should be called just once.
         */
        ap_log_perror(APLOG_MARK, APLOG_EMERG, 0, pPool,
                      "PyS60 initialized more than once.");

        rv = APR_EGENERAL;
    }

    return rv;
}


apr_status_t PyS60_InitializeThread()
{
    apr_status_t
        rv = APR_SUCCESS;

    if (!pthread_getspecific(is_initialized)) {
        /* The thread has not been initialized yet, so we need to do
         * it now.
         */
        pTheInterpreter->InitializeForeignThread();

        if (pthread_setspecific(is_initialized, pTheInterpreter) != 0) {
            ap_log_perror(APLOG_MARK, APLOG_EMERG, rv, 0, 
                          "Could not save thread-specific data.");
            
            rv = APR_EGENERAL;
        } 
    }

    return rv;
}


void PyS60_SetupThreadState(PyThreadState* pState)
{
    void
        *value = pthread_getspecific(is_initialized);
    
    if (value && (value != MAIN_THREAD_VALUE))
        PYTHON_TLS->thread_state = pState;
} 

} // extern "C"
