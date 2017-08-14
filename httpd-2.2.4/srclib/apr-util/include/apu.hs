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

/* 
 * Note: This is a Windows specific version of apu.h. It is renamed to
 * apu.h at the start of a Windows build.
 */
/* @file apu.h
 * @brief APR-Utility main file
 */

#ifdef __SYMBIAN32__
#ifndef APU_H
#define APU_H
/**
 * @defgroup APR_Util APR Utility Functions
 * @{
 */


/**
 * APU_DECLARE_EXPORT is defined when building the APR-UTIL dynamic library,
 * so that all public symbols are exported.
 *
 * APU_DECLARE_STATIC is defined when including the APR-UTIL public headers,
 * to provide static linkage when the dynamic library may be unavailable.
 *
 * APU_DECLARE_STATIC and APU_DECLARE_EXPORT are left undefined when
 * including the APR-UTIL public headers, to import and link the symbols from 
 * the dynamic APR-UTIL library and assure appropriate indirection and calling
 * conventions at compile time.
 */

#if !defined(__SYMBIAN32__)
/**
 * The public APR-UTIL functions are declared with APU_DECLARE(), so they may
 * use the most appropriate calling convention.  Public APR functions with 
 * variable arguments must use APU_DECLARE_NONSTD().
 *
 * @deffunc APU_DECLARE(rettype) apr_func(args);
 */
#define APU_DECLARE(type)            type
/**
 * The public APR-UTIL functions using variable arguments are declared with 
 * APU_DECLARE_NONSTD(), as they must use the C language calling convention.
 *
 * @deffunc APU_DECLARE_NONSTD(rettype) apr_func(args, ...);
 */
#define APU_DECLARE_NONSTD(type)     type
/**
 * The public APR-UTIL variables are declared with APU_DECLARE_DATA.
 * This assures the appropriate indirection is invoked at compile time.
 *
 * @deffunc APU_DECLARE_DATA type apr_variable;
 * @tip extern APU_DECLARE_DATA type apr_variable; syntax is required for
 * declarations within headers to properly import the variable.
 */
#define APU_DECLARE_DATA
#elif defined(APU_DECLARE_STATIC)
#define APU_DECLARE(type)            type __stdcall
#define APU_DECLARE_NONSTD(type)     type
#define APU_DECLARE_DATA
#elif defined(APU_DECLARE_EXPORT)
#define APU_DECLARE(type)            EXPORT_C type
#define APU_DECLARE_NONSTD(type)     EXPORT_C type
#define APU_DECLARE_DATA
#else
/**
 * The public APR-UTIL functions are declared with APU_DECLARE(), so they may
 * use the most appropriate calling convention.  Public APR functions with 
 * variable arguments must use APU_DECLARE_NONSTD().
 *
 */
#define APU_DECLARE(type)            IMPORT_C type
/**
 * The public APR-UTIL functions using variable arguments are declared with 
 * APU_DECLARE_NONSTD(), as they must use the C language calling convention.
 *
 */
#define APU_DECLARE_NONSTD(type)     IMPORT_C type
/**
 * The public APR-UTIL variables are declared with APU_DECLARE_DATA.
 * This assures the appropriate indirection is invoked at compile time.
 *
 * @remark extern APU_DECLARE_DATA type apr_variable; syntax is required for
 * declarations within headers to properly import the variable.
 */
#define APU_DECLARE_DATA

#endif
/** @} */
/*
 * we always have SDBM (it's in our codebase)
 */
#define APU_HAVE_SDBM   1
#define APU_HAVE_GDBM   0

#define APU_HAVE_DB     0


#define APU_HAVE_APR_ICONV     0
#define APU_HAVE_ICONV         0
#define APR_HAS_XLATE          (APU_HAVE_APR_ICONV || APU_HAVE_ICONV)

#define APU_HAVE_PGSQL      0
#define APU_HAVE_SQLITE2    0
#define APU_HAVE_SQLITE3    0

#if defined(APU_DECLARE_EXPORT)
#define APU_DECLARE_EXPORTED_DATA(type, name)\
APU_DECLARE_DATA extern type name

#define APU_DEFINE_EXPORTED_DATA_FUNC(type, name)\
APU_DECLARE(type*) _ ## name(void)\
{\
    return &name;\
}

#define APU_DEFINE_EXPORTED_DATA(type, name)\
APU_DEFINE_EXPORTED_DATA_FUNC(type, name)\
APU_DECLARE_DATA type name


#define APU_DECLARE_EXPORTED_DATA_ARRAY(type, name, dim)\
APU_DECLARE_DATA extern type name dim

#define APU_DEFINE_EXPORTED_DATA_ARRAY_FUNC(type, name, dim)\
APU_DECLARE(type) (* _ ## name(void)) dim\
{\
    return &name;\
}

#define APU_DEFINE_EXPORTED_DATA_ARRAY(type, name, dim)\
APU_DEFINE_EXPORTED_DATA_ARRAY_FUNC(type, name, dim)\
APU_DECLARE_DATA type name dim

#else
#define APU_DECLARE_EXPORTED_DATA(type, name)\
APU_DECLARE(type*) _ ## name(void)

#define APU_DECLARE_EXPORTED_DATA_ARRAY(type, name, dim)\
APU_DECLARE(type) (* _ ## name(void)) dim

/*
 * apr_hooks.h
 */
#define apr_hook_global_pool      (*_apr_hook_global_pool())
#define apr_hook_debug_enabled    (*_apr_hook_debug_enabled())
#define apr_hook_debug_current    (*_apr_hook_debug_current())
/*
 * apr_buckets.h
 */
#define apr_bucket_type_flush     (*_apr_bucket_type_flush())
#define apr_bucket_type_eos       (*_apr_bucket_type_eos())
#define apr_bucket_type_file      (*_apr_bucket_type_file())
#define apr_bucket_type_heap      (*_apr_bucket_type_heap())
#define apr_bucket_type_mmp       (*_apr_bucket_type_mmp())
#define apr_bucket_type_pool      (*_apr_bucket_type_pool())
#define apr_bucket_type_pipe      (*_apr_bucket_type_pipe())
#define apr_bucket_type_immortal  (*_apr_bucket_type_immortal())
#define apr_bucket_type_transient (*_apr_bucket_type_transient())
#define apr_bucket_type_socket    (*_apr_bucket_type_socket())

#endif

#endif /* APU_H */

#endif /* __SYMBIAN32__ */
