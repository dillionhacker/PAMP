#ifndef httpd_symbian_config_symbian_httpd_builtin_h
#define httpd_symbian_config_symbian_httpd_builtin_h
/* Copyright 2008 Nokia Corporation
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

//
// NOTE: This file is included by mmp-processor as well.
//

//
// If you want to have a module built-in, then turn the undef into
// a define.
//
// NOTE: This doesn't quite yet work from the top-level. Namely, 
//       when a module is built as a DLL, libhttpd must be built 
//       first so that the module can be linked. However, when a 
//       module is built as a static library and built into libhttpd, 
//       then the module must be built first, so that libhttpd can
//       be linked. So, when a module should be built into libhttpd,
//       you have to first build the module directly, and only then
//       build libhttpd.
//
// NOTE: This is primarily intended for making on-target debugging
//       easier. Normally there is NO reason to build in a module.
// 
// If you add something here, then please check
//
// - httpd-2.2/os/symbian/modules.c
// - httpd-2.2/build/libhttpd.mmp
//
// as well.

#undef HAVE_MOD_ALIAS_BUILTIN
#undef HAVE_MOD_ASIS_BUILTIN
#undef HAVE_MOD_AUTH_BASIC_BUILTIN
#undef HAVE_MOD_AUTH_DIGEST_BUILTIN
#undef HAVE_MOD_AUTHN_ALIAS_BUILTIN
#undef HAVE_MOD_AUTHN_ANON_BUILTIN
#undef HAVE_MOD_AUTHN_DBD_BUILTIN
#undef HAVE_MOD_AUTHN_DBM_BUILTIN
#undef HAVE_MOD_AUTHN_DEFAULT_BUILTIN
#undef HAVE_MOD_AUTHN_FILE_BUILTIN
#undef HAVE_MOD_AUTHNZ_LDAP_BUILTIN
#undef HAVE_MOD_AUTHZ_DBM_BUILTIN
#undef HAVE_MOD_AUTHZ_DEFAULT_BUILTIN
#undef HAVE_MOD_AUTHZ_GROUPFILE_BUILTIN
#undef HAVE_MOD_AUTHZ_HOST_BUILTIN
#undef HAVE_MOD_AUTHZ_OWNER_BUILTIN
#undef HAVE_MOD_AUTHZ_USER_BUILTIN
#undef HAVE_MOD_AUTOINDEX_BUILTIN
#undef HAVE_MOD_BUCKETEER_BUILTIN
#undef HAVE_MOD_CACHE_BUILTIN
#undef HAVE_MOD_CASE_FILTER_BUILTIN
#undef HAVE_MOD_CASE_FILTER_IN_BUILTIN
#undef HAVE_MOD_CERN_META_BUILTIN
#undef HAVE_MOD_CHARSET_LITE_BUILTIN
#undef HAVE_MOD_DAV_BUILTIN
#undef HAVE_MOD_DAV_FS_BUILTIN
#undef HAVE_MOD_DAV_LOCK_BUILTIN
#undef HAVE_MOD_DEFLATE_BUILTIN
#undef HAVE_MOD_DIR_BUILTIN
#undef HAVE_MOD_DUMPIO_BUILTIN
#undef HAVE_MOD_ECHO_BUILTIN
#undef HAVE_MOD_ENV_BUILTIN
#undef HAVE_MOD_EXAMPLE_BUILTIN
#undef HAVE_MOD_EXPIRES_BUILTIN
#undef HAVE_MOD_EXT_FILTER_BUILTIN
#undef HAVE_MOD_FILTER_BUILTIN
#undef HAVE_MOD_HEADERS_BUILTIN
#undef HAVE_MOD_IDENT_BUILTIN
#undef HAVE_MOD_IMAGEMAP_BUILTIN
#undef HAVE_MOD_INCLUDE_BUILTIN
#undef HAVE_MOD_INFO_BUILTIN
#undef HAVE_MOD_LOGIO_BUILTIN
#undef HAVE_MOD_LOG_CONFIG_BUILTIN
#undef HAVE_MOD_LOG_FORENSIC_BUILTIN
#undef HAVE_MOD_MIME_BUILTIN
#undef HAVE_MOD_MIME_MAGIC_BUILTIN
#undef HAVE_MOD_NEGOTIATION_BUILTIN
#undef HAVE_MOD_PROXY_BUILTIN
#undef HAVE_MOD_PROXY_AJP_BUILTIN
#undef HAVE_MOD_PROXY_BALANCER_BUILTIN
#undef HAVE_MOD_PROXY_CONNECT_BUILTIN
#undef HAVE_MOD_PROXY_FTP_BUILTIN
#undef HAVE_MOD_PROXY_HTTP_BUILTIN
#undef HAVE_MOD_REWRITE_BUILTIN
#undef HAVE_MOD_SETENVIF_BUILTIN
#undef HAVE_MOD_SPELING_BUILTIN
#undef HAVE_MOD_SSL_BUILTIN
#undef HAVE_MOD_STATUS_BUILTIN
#undef HAVE_MOD_UNIQUE_ID_BUILTIN
#undef HAVE_MOD_USERDIR_BUILTIN
#undef HAVE_MOD_USERTRACK_BUILTIN
#undef HAVE_MOD_VERSION_BUILTIN
#undef HAVE_MOD_VHOST_ALIAS_BUILTIN

#undef HAVE_MOD_SYMBIAN_BUILTIN
#undef HAVE_MOD_S60_BUILTIN

#endif
