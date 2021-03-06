#ifndef Mp_MOD_PYTHON_H
#define Mp_MOD_PYTHON_H

/*
 * Copyright 2004 Apache Software Foundation 
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You
 * may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 *
 * Originally developed by Gregory Trubetskoy.
 *
 *
 * mod_python.h 
 *
 * $Id: mod_python.h 467228 2006-10-24 03:41:54Z grahamd $
 *
 * See accompanying documentation and source code comments 
 * for details.
 *
 */

/*
 * NOTE - NOTE - NOTE - NOTE
 *
 * If you are looking at mod_python.h, it is an auto-generated file on
 * UNIX.  This file is kept around for the Win32 platform which
 * doesnot use autoconf. Any changes to mod_python.h must also be
 * reflected in mod_python.h.in.
 */


/* Apache headers */
#include "httpd.h"
#define CORE_PRIVATE
#include "http_config.h"
#include "http_core.h"
#include "http_main.h"
#include "http_connection.h"
#include "http_protocol.h"
#include "http_request.h"
#include "util_script.h"
#include "util_filter.h"
#include "http_log.h"
#include "apr_strings.h"
#include "apr_lib.h"
#include "apr_hash.h"
#include "apr_fnmatch.h"
#include "scoreboard.h"
#include "ap_mpm.h"
#include "ap_mmn.h"
#include "mod_include.h"
#if !defined(OS2) && !defined(WIN32) && !defined(BEOS) && !defined(NETWARE) &&\
    !defined(__SYMBIAN32__)
#include "unixd.h"
#endif

#if !AP_MODULE_MAGIC_AT_LEAST(20050127,0)
/* Debian backported ap_regex_t to Apache 2.0 and
 * thus made official version checking break. */
#ifndef AP_REG_EXTENDED
typedef regex_t ap_regex_t;
#define AP_REG_EXTENDED REG_EXTENDED
#define AP_REG_ICASE REG_ICASE
#endif
#endif

/* Python headers */
/* this gets rid of some compile warnings */
#if defined(_POSIX_THREADS)
#undef _POSIX_THREADS
#endif
#include "Python.h"
#include "structmember.h"

#if defined(WIN32) && !defined(WITH_THREAD)
#error Python threading must be enabled on Windows
#endif

#if !defined(WIN32)
#include <sys/socket.h>
#endif

/* pool given to us in ChildInit. We use it for 
   server.register_cleanup() */
extern apr_pool_t *child_init_pool;

/* Apache module declaration */
extern module AP_MODULE_DECLARE_DATA python_module;

#include "mpversion.h"
#include "util.h"
#include "hlist.h"
#include "hlistobject.h"
#include "tableobject.h"
#include "serverobject.h"
#include "connobject.h"
#include "_apachemodule.h"
#include "requestobject.h"
#include "filterobject.h"
#include "_pspmodule.h"
#include "finfoobject.h"

/** Things specific to mod_python, as an Apache module **/

#define MP_CONFIG_KEY "mod_python_config"
#define VERSION_COMPONENT "mod_python/" MPV_STRING
#define MODULENAME "mod_python.apache"
#define INITFUNC "init"
#define MAIN_INTERPRETER "main_interpreter"
#define FILTER_NAME "MOD_PYTHON"

/* used in python_directive_handler */
#define SILENT 1
#define NOTSILENT 0

/* MAX_LOCKS can now be set as a configure option
 * ./configure --with-max-locks=INTEGER
 */
#define MAX_LOCKS 8 

/* MUTEX_DIR can be set as a configure option
 * ./configure --with-mutex-dir=/path/to/dir
 */
#define MUTEX_DIR "/tmp" 

/* python 2.3 no longer defines LONG_LONG, it defines PY_LONG_LONG */
#ifndef LONG_LONG
#define LONG_LONG PY_LONG_LONG
#endif

/* structure to hold interpreter data */
typedef struct {
    PyInterpreterState *istate;
    PyObject *obcallback;
} interpreterdata;

/* global configuration parameters */
typedef struct
{
    apr_global_mutex_t **g_locks;
    int                  nlocks;
    int                  parent_pid;
} py_global_config;

/* structure describing per directory configuration parameters */
typedef struct {
    int           authoritative;
    char         *config_dir;
    apr_table_t  *directives;
    apr_table_t  *options;
    apr_hash_t   *hlists; /* hlists for every phase */
    apr_hash_t   *in_filters;
    apr_hash_t   *out_filters;
    apr_table_t  *imports;  /* for PythonImport */
} py_config;

/* register_cleanup info */
typedef struct
{
    request_rec  *request_rec;
    server_rec   *server_rec;
    PyObject     *handler;
    const char   *interpreter;
    PyObject     *data;
} cleanup_info;

/* request config structure */
typedef struct
{
    requestobject *request_obj;
    apr_hash_t    *dynhls;     /* dynamically registered handlers
                                  for this request */
    apr_hash_t   *in_filters;  /* dynamically registered input filters
                                  for this request */
    apr_hash_t   *out_filters; /* dynamically registered output filters
                                  for this request */

} py_req_config;

/* filter context */
typedef struct
{
    char *name;
    int transparent;
} python_filter_ctx;

/* a structure to hold a handler, 
   used in configuration for filters */
typedef struct
{
    char *handler;
    PyObject *callable;
    char *directory;
    int d_is_fnmatch;
    ap_regex_t *d_regex;
    char *location;
    int l_is_fnmatch;
    ap_regex_t *l_regex;
    hl_entry *parent;
} py_handler;

apr_status_t python_cleanup(void *data);
PyObject* python_interpreter_name(void);
requestobject *python_get_request_object(request_rec *req, const char *phase);

APR_DECLARE_OPTIONAL_FN(PyInterpreterState *, mp_acquire_interpreter, (const char *));
APR_DECLARE_OPTIONAL_FN(void, mp_release_interpreter, (void));
APR_DECLARE_OPTIONAL_FN(PyObject *, mp_get_request_object, (request_rec *));
APR_DECLARE_OPTIONAL_FN(PyObject *, mp_get_server_object, (server_rec *));
APR_DECLARE_OPTIONAL_FN(PyObject *, mp_get_connection_object, (conn_rec *));

#if defined(__SYMBIAN32__)

#define PyOS_AfterFork()

apr_status_t PyS60_Initialize(apr_pool_t* pPool);
apr_status_t PyS60_InitializeThread();
void         PyS60_SetupThreadState(PyThreadState*);

/*
 * FIXME: On EKA2 there appears to be a bug that prevents you from storing in 
 * FIXME: a static variable the address of a function in a DLL. If you do, the 
 * FIXME: loading of the application fails on target. A workaround is to have 
 * FIXME: a static function in between.
 * FIXME
 * FIXME: Thus, every Python function whose address is stored in a static
 * FIXME: structure must be wrapped by a local function whose address is
 * FIXME: stored instead.
 */
#define PyObject_GenericGetAttr _PyObject_GenericGetAttr 
#define PyObject_GenericSetAttr _PyObject_GenericSetAttr 

PyObject* _PyObject_GenericGetAttr(PyObject* a1, PyObject* a2);
int       _PyObject_GenericSetAttr(PyObject* a1, PyObject* a2, PyObject* a3);

#endif /* __SYMBIAN32__ */

#endif /* !Mp_MOD_PYTHON_H */

/*
# makes emacs go into C mode
### Local Variables:
### mode:c
### End:
*/
