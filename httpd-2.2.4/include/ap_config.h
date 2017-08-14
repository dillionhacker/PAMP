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

/**
 * @file ap_config.h
 * @brief Symbol export macros and hook functions
 */

#ifndef AP_CONFIG_H
#define AP_CONFIG_H

#include "apr.h"
#include "apr_hooks.h"
#include "apr_optional_hooks.h"

/* Although this file doesn't declare any hooks, declare the hook group here */
/** 
 * @defgroup hooks Apache Hooks 
 * @ingroup  APACHE_CORE
 */

#ifdef DOXYGEN
/* define these just so doxygen documents them */

/**
 * AP_DECLARE_STATIC is defined when including Apache's Core headers,
 * to provide static linkage when the dynamic library may be unavailable.
 *
 * @see AP_DECLARE_EXPORT
 *
 * AP_DECLARE_STATIC and AP_DECLARE_EXPORT are left undefined when
 * including Apache's Core headers, to import and link the symbols from the 
 * dynamic Apache Core library and assure appropriate indirection and calling 
 * conventions at compile time.
 */
# define AP_DECLARE_STATIC
/**
 * AP_DECLARE_EXPORT is defined when building the Apache Core dynamic
 * library, so that all public symbols are exported.
 *
 * @see AP_DECLARE_STATIC
 */
# define AP_DECLARE_EXPORT

#endif /* def DOXYGEN */

#if defined(__SYMBIAN32__)

#if defined(AP_DECLARE_STATIC)
#define AP_DECLARE(type)            type __stdcall
#define AP_DECLARE_NONSTD(type)     type
#define AP_DECLARE_DATA
#elif defined(AP_DECLARE_EXPORT)
#define AP_DECLARE(type)            EXPORT_C type
#define AP_DECLARE_NONSTD(type)     EXPORT_C type
#define AP_DECLARE_DATA
#else
#define AP_DECLARE(type)            IMPORT_C type
#define AP_DECLARE_NONSTD(type)     IMPORT_C type
#define AP_DECLARE_DATA

#endif

#elif !defined(WIN32)
/**
 * Apache Core dso functions are declared with AP_DECLARE(), so they may
 * use the most appropriate calling convention.  Hook functions and other
 * Core functions with variable arguments must use AP_DECLARE_NONSTD().
 * @code
 * AP_DECLARE(rettype) ap_func(args)
 * @endcode
 */
#define AP_DECLARE(type)            type

/**
 * Apache Core dso variable argument and hook functions are declared with 
 * AP_DECLARE_NONSTD(), as they must use the C language calling convention.
 * @see AP_DECLARE
 * @code
 * AP_DECLARE_NONSTD(rettype) ap_func(args [...])
 * @endcode
 */
#define AP_DECLARE_NONSTD(type)     type

/**
 * Apache Core dso variables are declared with AP_MODULE_DECLARE_DATA.
 * This assures the appropriate indirection is invoked at compile time.
 *
 * @note AP_DECLARE_DATA extern type apr_variable; syntax is required for
 * declarations within headers to properly import the variable.
 * @code
 * AP_DECLARE_DATA type apr_variable
 * @endcode
 */
#define AP_DECLARE_DATA

#elif defined(AP_DECLARE_STATIC)
#define AP_DECLARE(type)            type __stdcall
#define AP_DECLARE_NONSTD(type)     type
#define AP_DECLARE_DATA
#elif defined(AP_DECLARE_EXPORT)
#define AP_DECLARE(type)            __declspec(dllexport) type __stdcall
#define AP_DECLARE_NONSTD(type)     __declspec(dllexport) type
#define AP_DECLARE_DATA             __declspec(dllexport)
#else
#define AP_DECLARE(type)            __declspec(dllimport) type __stdcall
#define AP_DECLARE_NONSTD(type)     __declspec(dllimport) type
#define AP_DECLARE_DATA             __declspec(dllimport)
#endif

#if !defined(WIN32) || defined(AP_MODULE_DECLARE_STATIC)
/**
 * Declare a dso module's exported module structure as AP_MODULE_DECLARE_DATA.
 *
 * Unless AP_MODULE_DECLARE_STATIC is defined at compile time, symbols 
 * declared with AP_MODULE_DECLARE_DATA are always exported.
 * @code
 * module AP_MODULE_DECLARE_DATA mod_tag
 * @endcode
 */
#if defined(WIN32)
#define AP_MODULE_DECLARE(type)            type __stdcall
#else
#define AP_MODULE_DECLARE(type)            type
#endif
#define AP_MODULE_DECLARE_NONSTD(type)     type
#define AP_MODULE_DECLARE_DATA
#else
/**
 * AP_MODULE_DECLARE_EXPORT is a no-op.  Unless contradicted by the
 * AP_MODULE_DECLARE_STATIC compile-time symbol, it is assumed and defined.
 *
 * The old SHARED_MODULE compile-time symbol is now the default behavior, 
 * so it is no longer referenced anywhere with Apache 2.0.
 */
#define AP_MODULE_DECLARE_EXPORT
#define AP_MODULE_DECLARE(type)          __declspec(dllexport) type __stdcall
#define AP_MODULE_DECLARE_NONSTD(type)   __declspec(dllexport) type
#define AP_MODULE_DECLARE_DATA           __declspec(dllexport)
#endif

/**
 * Declare a hook function
 * @param ret The return type of the hook
 * @param name The hook's name (as a literal)
 * @param args The arguments the hook function takes, in brackets.
 */
#define AP_DECLARE_HOOK(ret,name,args) \
	APR_DECLARE_EXTERNAL_HOOK(ap,AP,ret,name,args)

/** @internal */
#define AP_IMPLEMENT_HOOK_BASE(name) \
	APR_IMPLEMENT_EXTERNAL_HOOK_BASE(ap,AP,name)

/**
 * Implement an Apache core hook that has no return code, and
 * therefore runs all of the registered functions. The implementation
 * is called ap_run_<i>name</i>.
 *
 * @param name The name of the hook
 * @param args_decl The declaration of the arguments for the hook, for example
 * "(int x,void *y)"
 * @param args_use The arguments for the hook as used in a call, for example
 * "(x,y)"
 * @note If IMPLEMENTing a hook that is not linked into the Apache core,
 * (e.g. within a dso) see APR_IMPLEMENT_EXTERNAL_HOOK_VOID.
 */
#define AP_IMPLEMENT_HOOK_VOID(name,args_decl,args_use) \
	APR_IMPLEMENT_EXTERNAL_HOOK_VOID(ap,AP,name,args_decl,args_use)

/**
 * Implement an Apache core hook that runs until one of the functions
 * returns something other than ok or decline. That return value is
 * then returned from the hook runner. If the hooks run to completion,
 * then ok is returned. Note that if no hook runs it would probably be
 * more correct to return decline, but this currently does not do
 * so. The implementation is called ap_run_<i>name</i>.
 *
 * @param ret The return type of the hook (and the hook runner)
 * @param name The name of the hook
 * @param args_decl The declaration of the arguments for the hook, for example
 * "(int x,void *y)"
 * @param args_use The arguments for the hook as used in a call, for example
 * "(x,y)"
 * @param ok The "ok" return value
 * @param decline The "decline" return value
 * @return ok, decline or an error.
 * @note If IMPLEMENTing a hook that is not linked into the Apache core,
 * (e.g. within a dso) see APR_IMPLEMENT_EXTERNAL_HOOK_RUN_ALL.
 */
#define AP_IMPLEMENT_HOOK_RUN_ALL(ret,name,args_decl,args_use,ok,decline) \
	APR_IMPLEMENT_EXTERNAL_HOOK_RUN_ALL(ap,AP,ret,name,args_decl, \
                                            args_use,ok,decline)

/**
 * Implement a hook that runs until a function returns something other than 
 * decline. If all functions return decline, the hook runner returns decline. 
 * The implementation is called ap_run_<i>name</i>.
 *
 * @param ret The return type of the hook (and the hook runner)
 * @param name The name of the hook
 * @param args_decl The declaration of the arguments for the hook, for example
 * "(int x,void *y)"
 * @param args_use The arguments for the hook as used in a call, for example
 * "(x,y)"
 * @param decline The "decline" return value
 * @return decline or an error.
 * @note If IMPLEMENTing a hook that is not linked into the Apache core
 * (e.g. within a dso) see APR_IMPLEMENT_EXTERNAL_HOOK_RUN_FIRST.
 */
#define AP_IMPLEMENT_HOOK_RUN_FIRST(ret,name,args_decl,args_use,decline) \
	APR_IMPLEMENT_EXTERNAL_HOOK_RUN_FIRST(ap,AP,ret,name,args_decl, \
                                              args_use,decline)

/* Note that the other optional hook implementations are straightforward but
 * have not yet been needed
 */

/**
 * Implement an optional hook. This is exactly the same as a standard hook
 * implementation, except the hook is optional.
 * @see AP_IMPLEMENT_HOOK_RUN_ALL
 */
#define AP_IMPLEMENT_OPTIONAL_HOOK_RUN_ALL(ret,name,args_decl,args_use,ok, \
					   decline) \
	APR_IMPLEMENT_OPTIONAL_HOOK_RUN_ALL(ap,AP,ret,name,args_decl, \
                                            args_use,ok,decline)

/**
 * Hook an optional hook. Unlike static hooks, this uses a macro instead of a
 * function.
 */
#define AP_OPTIONAL_HOOK(name,fn,pre,succ,order) \
        APR_OPTIONAL_HOOK(ap,name,fn,pre,succ,order)

#include "os.h"
#if !defined(WIN32) && !defined(NETWARE)
#include "ap_config_auto.h"
#include "ap_config_layout.h"
#endif
#if defined(NETWARE)
#define AP_NONBLOCK_WHEN_MULTI_LISTEN 1
#endif

/* TODO - We need to put OS detection back to make all the following work */

#if defined(SUNOS4) || defined(IRIX) || defined(NEXT) || defined(AUX3) \
    || defined (UW) || defined(LYNXOS) || defined(TPF) || defined(__SYMBIAN32__)
/* These systems don't do well with any lingering close code; I don't know
 * why -- manoj */
#define NO_LINGCLOSE
#endif

/* If APR has OTHER_CHILD logic, use reliable piped logs. */
#if APR_HAS_OTHER_CHILD
#define AP_HAVE_RELIABLE_PIPED_LOGS TRUE
#endif

/* Presume that the compiler supports C99-style designated
 * initializers if using GCC (but not G++), or for any other compiler
 * which claims C99 support. */
#if (defined(__GNUC__) && !defined(__cplusplus))                \
     || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define AP_HAVE_DESIGNATED_INITIALIZER
#endif


#if defined(__SYMBIAN32__)

#if !defined(NO_SYMBIAN_EXPORT_BY_NAME)
/*
 * Symbian exports by ordinal and not by symbol. So as not to require too 
 * much modifications in existing modules we implement a custom export by
 * name ordinal.
 */

typedef struct ap_export_entry
{
    const char* const name;
    void* const       symbol;
} ap_export_entry;


#if defined(__cplusplus)
extern "C"
{
#endif

AP_DECLARE(int) ap_lookup_table_symbol(const ap_export_entry* entries,
                                       int                    count,
                                       const char*            name,
                                       void**                 symbol);

#define AP_EXPORTS_BEGIN() \
static const ap_export_entry ap_export_entries[] =\
{

#define AP_EXPORT_SYMBOL(symbol) { #symbol, &symbol },

#define AP_EXPORTS_END() \
};\
\
static const int ap_export_entries_count = \
sizeof(ap_export_entries)/sizeof(ap_export_entries[0]);\
\
EXPORT_C int ap_lookup_symbol(const char* name, void** symbol)\
{\
return ap_lookup_table_symbol(ap_export_entries,\
                              ap_export_entries_count,\
                              name,\
                              symbol);\
}

#define AP_EXPORT_1_SYMBOL(symbol)\
AP_EXPORTS_BEGIN()\
AP_EXPORT_SYMBOL(symbol)\
AP_EXPORTS_END()

#if defined(__cplusplus)
}
#endif

#else /* NO_SYMBIAN_EXPORT_BY_NAME */

#define AP_EXPORT_1_SYMBOL(symbol)
#define AP_EXPORTS_BEGIN() 
#define AP_EXPORT_SYMBOL() 
#define AP_EXPORTS_END() 

#endif


#if defined(AP_DECLARE_EXPORT)
#define AP_DECLARE_EXPORTED_DATA(type, name)\
AP_DECLARE_DATA extern type name

#define AP_DEFINE_EXPORTED_DATA_FUNC(type, name)\
AP_DECLARE(type*) _ ## name(void)\
{\
    return &name;\
}

#define AP_DEFINE_EXPORTED_DATA(type, name)\
AP_DEFINE_EXPORTED_DATA_FUNC(type, name)\
AP_DECLARE_DATA type name


#define AP_DECLARE_EXPORTED_DATA_ARRAY(type, name, dim)\
AP_DECLARE_DATA extern type name dim

#define AP_DEFINE_EXPORTED_DATA_ARRAY_FUNC(type, name, dim)\
AP_DECLARE(type) (* _ ## name(void)) dim\
{\
    return &name;\
}

#define AP_DEFINE_EXPORTED_DATA_ARRAY(type, name, dim)\
AP_DEFINE_EXPORTED_DATA_ARRAY_FUNC(type, name, dim)\
AP_DECLARE_DATA type name dim

#else

#define AP_DECLARE_EXPORTED_DATA(type, name)\
AP_DECLARE(type*) _ ## name(void)

#define AP_DECLARE_EXPORTED_DATA_ARRAY(type, name, dim)\
AP_DECLARE(type) (* _ ## name(void)) dim

/*
 * ap_listen.h
 */
#define ap_listeners                 (*_ap_listeners())

/*
 * http_config.h
 */
#define ap_top_module                (*_ap_top_module())
#define ap_prelinked_modules         (*_ap_prelinked_modules())
#define ap_prelinked_module_symbols  (*_ap_prelinked_module_symbols())
#define ap_preloaded_modules         (*_ap_preloaded_modules())
#define ap_loaded_modules            (*_ap_loaded_modules())

/*
 * http_connection.h
 */
#define ap_bucket_type_eoc           (*_ap_bucket_type_eoc())

/*
 * http_core.h
 */
#define core_module                  (*_core_module())
#define ap_subreq_core_filter_handle (*_ap_subreq_core_filter_handle())
#define ap_core_output_filter_handle (*_ap_core_output_filter_handle())
#define ap_content_length_filter     (*_ap_content_length_filter())
#define ap_core_input_filter_handle  (*_ap_core_input_filter_handle())

/*
 * http_log.h
 */
#define ap_default_loglevel          (*_ap_default_loglevel())
/*
 * http_main.h
 */
#define ap_server_argv0              (*_ap_server_argv0())
#define ap_server_root               (*_ap_server_root())
#define ap_server_pre_read_config    (*_ap_server_pre_read_config())
#define ap_server_post_read_config   (*_ap_server_post_read_config())
#define ap_server_config_defines     (*_ap_server_config_defines())

/*
 * http_protocol.h
 */
#define ap_old_write_func            (*_ap_old_write_func())
#define ap_bucket_type_error         (*_ap_bucket_type_error())

/*
 * scoreboard.h
 */
#define ap_scoreboard_image          (*_ap_scoreboard_image())
#define ap_scoreboard_fname          (*_ap_scoreboard_fname())
#define ap_extended_status           (*_ap_extended_status())
#define ap_my_generation             (*_ap_my_generation())

/*
 * util_cfgtree.h
 */
#define ap_conftree                  (*_ap_conftree())

#endif

#else
/* NOT __SYMBIAN32__ */

#define AP_EXPORTS_BEGIN()
#define AP_EXPORT_SYMBOL()
#define AP_EXPORTS_END()
#define AP_EXPORT_1_SYMBOL(symbol)

#define AP_DECLARE_EXPORTED_DATA(type, name)\
AP_DECLARE_DATA extern type name

#define AP_DEFINE_EXPORTED_DATA(type, name)\
AP_DECLARE_DATA type name

#define AP_DECLARE_EXPORTED_DATA_ARRAY(type, name, dim)\
AP_DECLARE_DATA extern type name dim

#define AP_DEFINE_EXPORTED_DATA_ARRAY(type, name, dim)\
AP_DECLARE_DATA type name dim

#endif

#endif /* AP_CONFIG_H */
