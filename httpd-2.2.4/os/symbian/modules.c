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

/* modules.c --- major modules compiled into Apache for Symbian.
 */

#define CORE_PRIVATE
#include "httpd.h"
#include "http_config.h"
#include "../../symbian/config/symbian_httpd_builtin.h"


extern module core_module;
extern module http_module;
extern module mpm_symbian_module;

extern module so_module;

extern module alias_module;
extern module asis_module;
extern module auth_basic_module;
extern module auth_digest_module;
extern module authn_alias_module;
extern module authn_anon_module;
extern module authn_dbd_module;
extern module authn_dbm_module;
extern module authn_default_module;
extern module authn_file_module;
extern module authnz_ldap_module;
extern module authz_dbm_module;
extern module authz_default_module;
extern module authz_groupfile_module;
extern module authz_host_module;
extern module authz_user_module;
extern module autoindex_module;
extern module bucketeer_module;
extern module cache_module;
extern module case_filter_module;
extern module case_filter_in_module;
extern module cern_meta_module;
extern module charset_lite_module;
extern module dav_module;
extern module dav_fs_module;
extern module dav_lock_module;
extern module deflate_module;
extern module dir_module;
extern module dumpio_module;
extern module echo_module;
extern module env_module;
extern module example_module;
extern module expires_module;
extern module ext_filter_module;
extern module filter_module;
extern module headers_module;
extern module ident_module;
extern module imagemap_module;
extern module include_module;
extern module info_module;
extern module logio_module;
extern module log_config_module;
extern module log_forensic_module;
extern module mime_module;
extern module mime_magic_module;
extern module negotiation_module;
extern module proxy_module;
extern module proxy_ajp_module;
extern module proxy_balancer_module;
extern module proxy_connect_module;
extern module proxy_ftp_module;
extern module proxy_http_module;
extern module rewrite_module;
extern module setenvif_module;
extern module speling_module;
extern module ssl_module;
extern module status_module;
extern module unique_id_module;
extern module userdir_module;
extern module usertrack_module;
extern module version_module;
extern module vhost_alias_module;

extern module symbian_calendar_module;
extern module symbian_contacts_module;
extern module symbian_dos_module;
extern module symbian_inbox_module;
extern module symbian_logs_module;
extern module symbian_messages_module;
extern module symbian_proximity_module;

extern module s60_camera_module;
extern module s60_favourites_module;
extern module s60_im_module;


AP_DEFINE_EXPORTED_DATA_ARRAY(module *, ap_prelinked_modules, []) = {
  &core_module,
  &http_module,
  &mpm_symbian_module,
  &so_module,

#if defined(HAVE_MOD_ALIAS_BUILTIN)
  &alias_module,
#endif

#if defined(HAVE_MOD_ASIS_BUILTIN)
  &asis_module,
#endif

#if defined(HAVE_MOD_AUTH_BASIC_BUILTIN)
  &auth_basic_module,
#endif

#if defined(HAVE_MOD_AUTH_DIGEST_BUILTIN)
  &auth_digest_module,
#endif

#if defined(HAVE_MOD_AUTHN_ALIAS_BUILTIN)
  &authn_alias_module,
#endif

#if defined(HAVE_MOD_AUTHN_ANON_BUILTIN)
  &authn_anon_module,
#endif

#if defined(HAVE_MOD_AUTHN_DBD_BUILTIN)
  &authn_dbd_module,
#endif

#if defined(HAVE_MOD_AUTHN_DBM_BUILTIN)
  &authn_dbm_module,
#endif

#if defined(HAVE_MOD_AUTHN_DEFAULT_BUILTIN)
  &authn_default_module,
#endif

#if defined(HAVE_MOD_AUTHN_FILE_BUILTIN)
  &authn_file_module,
#endif

#if defined(HAVE_MOD_AUTHNZ_LDAP_BUILTIN)
  &authnz_ldap_module,
#endif

#if defined(HAVE_MOD_AUTHZ_DBM_BUILTIN)
  &authz_dbm_module,
#endif

#if defined(HAVE_MOD_AUTHZ_DEFAULT_BUILTIN)
  &authz_default_module,
#endif

#if defined(HAVE_MOD_AUTHZ_GROUPFILE_BUILTIN)
  &authz_groupfile_module,
#endif

#if defined(HAVE_MOD_AUTHZ_HOST_BUILTIN)
  &authz_host_module,
#endif

#if defined(HAVE_MOD_AUTHZ_USER_BUILTIN)
  &authz_user_module,
#endif

#if defined(HAVE_MOD_AUTOINDEX_BUILTIN)
  &autoindex_module,
#endif

#if defined(HAVE_MOD_BUCKETEER_BUILTIN)
  &bucketeer_module,
#endif

#if defined(HAVE_MOD_CACHE_BUILTIN)
  &cache_module,
#endif

#if defined(HAVE_MOD_CASE_FILTER_BUILTIN)
  &case_filter_module,
#endif

#if defined(HAVE_MOD_CASE_FILTER_IN_BUILTIN)
  &case_filter_in_module,
#endif

#if defined(HAVE_MOD_CERN_META_BUILTIN)
  &cern_meta_module,
#endif

#if defined(HAVE_MOD_CHARSET_LITE_BUILTIN)
  &charset_lite_module,
#endif

#if defined(HAVE_MOD_DAV_BUILTIN)
  &dav_module,
#endif

#if defined(HAVE_MOD_DAV_FS_BUILTIN)
  &dav_fs_module,
#endif

#if defined(HAVE_MOD_DAV_LOCK_BUILTIN)
  &dav_lock_module,
#endif

#if defined(HAVE_MOD_DEFLATE_BUILTIN)
  &deflate_module,
#endif

#if defined(HAVE_MOD_DIR_BUILTIN)
  &dir_module,
#endif

#if defined(HAVE_MOD_DUMPIO_BUILTIN)
  &dumpio_module,
#endif

#if defined(HAVE_MOD_ECHO_BUILTIN)
  &echo_module,
#endif

#if defined(HAVE_MOD_ENV_BUILTIN)
  &env_module,
#endif

#if defined(HAVE_MOD_EXAMPLE_BUILTIN)
  &exmaple_module,
#endif

#if defined(HAVE_MOD_EXPIRES_BUILTIN)
  &expires_module,
#endif

#if defined(HAVE_MOD_EXT_FILTER_BUILTIN)
  &ext_filter_module,
#endif

#if defined(HAVE_MOD_FILTER_BUILTIN)
  &filter_module,
#endif

#if defined(HAVE_MOD_HEADERS_BUILTIN)
  &headers_module,
#endif

#if defined(HAVE_MOD_IDENT_BUILTIN)
  &ident_module,
#endif

#if defined(HAVE_MOD_IMAGEMAP_BUILTIN)
  &imagemap_module,
#endif

#if defined(HAVE_MOD_INCLUDE_BUILTIN)
  &include_module,
#endif

#if defined(HAVE_MOD_INFO_BUILTIN)
  &info_module,
#endif

#if defined(HAVE_MOD_LOGIO_BUILTIN)
  &logio_module,
#endif

#if defined(HAVE_MOD_LOG_CONFIG_BUILTIN)
  &log_config_module,
#endif

#if defined(HAVE_MOD_LOG_FORENSIC_BUILTIN)
  &log_forensic_module,
#endif

#if defined(HAVE_MOD_MIME_BUILTIN)
  &mime_module,
#endif

#if defined(HAVE_MOD_MIME_MAGIC_BUILTIN)
  &mime_magic_module,
#endif

#if defined(HAVE_MOD_NEGOTIATION_BUILTIN)
  &negotiation_module,
#endif

#if defined(HAVE_MOD_PROXY_BUILTIN)
  &proxy_module,
#endif

#if defined(HAVE_MOD_PROXY_AJP_BUILTIN)
  &proxy_ajp_module,
#endif

#if defined(HAVE_MOD_PROXY_BALANCER_BUILTIN)
  &proxy_balancer_module,
#endif

#if defined(HAVE_MOD_PROXY_CONNECT_BUILTIN)
  &proxy_connect_module,
#endif

#if defined(HAVE_MOD_PROXY_FTP_BUILTIN)
  &proxy_ftp_module,
#endif

#if defined(HAVE_MOD_PROXY_HTTP_BUILTIN)
  &proxy_http_module,
#endif

#if defined(HAVE_MOD_REWRITE_BUILTIN)
  &rewrite_module,
#endif

#if defined(HAVE_MOD_SETENVIF_BUILTIN)
  &setenvif_module,
#endif

#if defined(HAVE_MOD_SPELING_BUILTIN)
  &speling_module,
#endif

#if defined(HAVE_MOD_SSL_BUILTIN)
  &ssl_module,
#endif

#if defined(HAVE_MOD_STATUS_BUILTIN)
  &status_module,
#endif

#if defined(HAVE_MOD_UNIQUE_ID_BUILTIN)
  &unique_id_module,
#endif

#if defined(HAVE_MOD_USERDIR_BUILTIN)
  &userdir_module,
#endif

#if defined(HAVE_MOD_USERTRACK_BUILTIN)
  &usertrack_module,
#endif

#if defined(HAVE_MOD_VERSION_BUILTIN)
  &version_module,
#endif

#if defined(HAVE_MOD_VHOST_ALIAS_BUILTIN)
  &vhost_alias_module,
#endif

#if defined(HAVE_MOD_SYMBIAN_BUILTIN)
  &symbian_calendar_module,
  &symbian_contacts_module,
  &symbian_dos_module,
  &symbian_inbox_module,
  &symbian_logs_module,
  &symbian_messages_module,
  &symbian_proximity_module,
#endif

#if defined(HAVE_MOD_S60_BUILTIN)
  &s60_camera_module,
  &s60_favourites_module,
  &s60_im_module,
#endif

  NULL
};


AP_DEFINE_EXPORTED_DATA_ARRAY(ap_module_symbol_t, 
                              ap_prelinked_module_symbols,
                              []) = {
  { "core_module", &core_module },
  { "http_module", &http_module },
  { "mpm_symbian_module", &mpm_symbian_module },

#if defined(HAVE_MOD_SO_BUILTIN)
  { "so_module", &so_module },
#endif

#if defined(HAVE_MOD_ALIAS_BUILTIN)
  { "alias_module", &alias_module },
#endif

#if defined(HAVE_MOD_ASIS_BUILTIN)
  { "asis_module", &asis_module },
#endif

#if defined(HAVE_MOD_AUTH_BASIC_BUILTIN)
  { "auth_basic_module", &auth_basic_module },
#endif

#if defined(HAVE_MOD_AUTH_DIGEST_BUILTIN)
  { "auth_digest_module", &auth_digest_module },
#endif

#if defined(HAVE_MOD_AUTHN_ALIAS_BUILTIN)
  { "authn_alias_module", &authn_alias_module },
#endif

#if defined(HAVE_MOD_AUTHN_ANON_BUILTIN)
  { "authn_anon_module", &authn_anon_module },
#endif

#if defined(HAVE_MOD_AUTHN_DBD_BUILTIN)
  { "authn_dbd_module", &authn_dbd_module },
#endif

#if defined(HAVE_MOD_AUTHN_DBM_BUILTIN)
  { "authn_dbm_module", &authn_dbm_module },
#endif

#if defined(HAVE_MOD_AUTHN_DEFAULT_BUILTIN)
  { "authn_default_module", &authn_default_module },
#endif

#if defined(HAVE_MOD_AUTHN_FILE_BUILTIN)
  { "authn_file_module", &authn_file_module },
#endif

#if defined(HAVE_MOD_AUTHNZ_LDAP_BUILTIN)
  { "authnz_ldap_module", &authnz_ldap_module },
#endif

#if defined(HAVE_MOD_AUTHZ_DBM_BUILTIN)
  { "authz_dbm_module", &authz_dbm_module },
#endif

#if defined(HAVE_MOD_AUTHZ_DEFAULT_BUILTIN)
  { "authz_default_module", &authz_default_module },
#endif

#if defined(HAVE_MOD_AUTHZ_GROUPFILE_BUILTIN)
  { "authz_groupfile_module", &authz_groupfile_module },
#endif

#if defined(HAVE_MOD_AUTHZ_HOST_BUILTIN)
  { "authz_host_module", &authz_host_module },
#endif

#if defined(HAVE_MOD_AUTHZ_USER_BUILTIN)
  { "authz_user_module", &authz_user_module },
#endif

#if defined(HAVE_MOD_AUTOINDEX_BUILTIN)
  { "autoindex_module", &autoindex_module },
#endif

#if defined(HAVE_MOD_BUCKETEER_BUILTIN)
  { "bucketeer_module", &bucketeer_module },
#endif

#if defined(HAVE_MOD_CACHE_BUILTIN)
  { "cache_module", &cache_module },
#endif

#if defined(HAVE_MOD_CASE_FILTER_BUILTIN)
  { "case_filter_module", &case_filter_module },
#endif

#if defined(HAVE_MOD_CASE_FILTER_IN_BUILTIN)
  { "case_filter_in_module", &case_filter_in_module },
#endif

#if defined(HAVE_MOD_CERN_META_BUILTIN)
  { "cern_meta_module", &cern_meta_module },
#endif

#if defined(HAVE_MOD_CHARSET_LITE_BUILTIN)
  { "charset_lite_module", &charset_lite_module },
#endif

#if defined(HAVE_MOD_DAV_BUILTIN)
  { "dav_module", &dav_module },
#endif

#if defined(HAVE_MOD_DAV_FS_BUILTIN)
  { "dav_fs_module", &dav_fs_module },
#endif

#if defined(HAVE_MOD_DAV_LOCK_BUILTIN)
  { "dav_lock_module", &dav_lock_module },
#endif

#if defined(HAVE_MOD_DEFLATE_BUILTIN)
  { "deflate_module", &deflate_module },
#endif

#if defined(HAVE_MOD_DIR_BUILTIN)
  { "dir_module", &dir_module },
#endif

#if defined(HAVE_MOD_DUMPIO_BUILTIN)
  { "dumpio_module", &dumpio_module },
#endif

#if defined(HAVE_MOD_ECHO_BUILTIN)
  { "echo_module", &echo_module },
#endif

#if defined(HAVE_MOD_ENV_BUILTIN)
  { "env_module", &env_module },
#endif

#if defined(HAVE_MOD_EXAMPLE_BUILTIN)
  { "exmaple_module", &exmaple_module },
#endif

#if defined(HAVE_MOD_EXPIRES_BUILTIN)
  { "expires_module", &expires_module },
#endif

#if defined(HAVE_MOD_EXT_FILTER_BUILTIN)
  { "ext_filter_module", &ext_filter_module },
#endif

#if defined(HAVE_MOD_FILTER_BUILTIN)
  { "filter_module", &filter_module },
#endif

#if defined(HAVE_MOD_HEADERS_BUILTIN)
  { "headers_module", &headers_module },
#endif

#if defined(HAVE_MOD_IDENT_BUILTIN)
  { "ident_module", &ident_module },
#endif

#if defined(HAVE_MOD_IMAGEMAP_BUILTIN)
  { "imagemap_module", &imagemap_module },
#endif

#if defined(HAVE_MOD_INCLUDE_BUILTIN)
  { "include_module", &include_module },
#endif

#if defined(HAVE_MOD_INFO_BUILTIN)
  { "info_module", &info_module },
#endif

#if defined(HAVE_MOD_LOGIO_BUILTIN)
  { "logio_module", &logio_module },
#endif

#if defined(HAVE_MOD_LOG_CONFIG_BUILTIN)
  { "log_config_module", &log_config_module },
#endif

#if defined(HAVE_MOD_LOG_FORENSIC_BUILTIN)
  { "log_forensic_module", &log_forensic_module },
#endif

#if defined(HAVE_MOD_MIME_BUILTIN)
  { "mime_module", &mime_module },
#endif

#if defined(HAVE_MOD_MIME_MAGIC_BUILTIN)
  { "mime_magic_module", &mime_magic_module },
#endif

#if defined(HAVE_MOD_NEGOTIATION_BUILTIN)
  { "negotiation_module", &negotiation_module },
#endif

#if defined(HAVE_MOD_PROXY_BUILTIN)
  { "proxy_module", &proxy_module },
#endif

#if defined(HAVE_MOD_PROXY_AJP_BUILTIN)
  { "proxy_ajp_module", &proxy_ajp_module },
#endif

#if defined(HAVE_MOD_PROXY_BALANCER_BUILTIN)
  { "proxy_balancer_module", &proxy_balancer_module },
#endif

#if defined(HAVE_MOD_PROXY_CONNECT_BUILTIN)
  { "proxy_connect_module", &proxy_connect_module },
#endif

#if defined(HAVE_MOD_PROXY_FTP_BUILTIN)
  { "proxy_ftp_module", &proxy_ftp_module },
#endif

#if defined(HAVE_MOD_PROXY_HTTP_BUILTIN)
  { "proxy_http_module", &proxy_http_module },
#endif

#if defined(HAVE_MOD_REWRITE_BUILTIN)
  { "rewrite_module", &rewrite_module },
#endif

#if defined(HAVE_MOD_SETENVIF_BUILTIN)
  { "setenvif_module", &setenvif_module },
#endif

#if defined(HAVE_MOD_SPELING_BUILTIN)
  { "speling_module", &speling_module },
#endif

#if defined(HAVE_MOD_SSL_BUILTIN)
  { "ssl_module", &ssl_module },
#endif

#if defined(HAVE_MOD_STATUS_BUILTIN)
  { "status_module", &status_module },
#endif

#if defined(HAVE_MOD_UNIQUE_ID_BUILTIN)
  { "unique_id_module", &unique_id_module },
#endif

#if defined(HAVE_MOD_USERDIR_BUILTIN)
  { "userdir_module", &userdir_module },
#endif

#if defined(HAVE_MOD_USERTRACK_BUILTIN)
  { "usertrack_module", &usertrack_module },
#endif

#if defined(HAVE_MOD_VERSION_BUILTIN)
  { "version_module", &version_module },
#endif

#if defined(HAVE_MOD_VHOST_ALIAS_BUILTIN)
  { "vhost_alias_module", &vhost_alias_module },
#endif

#if defined(HAVE_MOD_SYMBIAN_BUILTIN)
  { "symbian_calendar_module", &symbian_calendar_module },
  { "symbian_contacts_module", &symbian_contacts_module },
  { "symbian_dos_module", &symbian_dos_module },
  { "symbian_inbox_module", &symbian_inbox_module },
  { "symbian_logs_module", &symbian_logs_module },
  { "symbian_messages_module", &symbian_messages_module },
  { "symbian_proximity_module", &symbian_proximity_module },
#endif

#if defined(HAVE_MOD_S60_BUILTIN)
  { "s60_camera_module", &s60_camera_module },
  { "s60_favourites_module", &s60_favourites_module },
  { "s60_im_module", &s60_im_module },
#endif

  NULL
};


AP_DEFINE_EXPORTED_DATA_ARRAY(module *, ap_preloaded_modules, []) = {
  &core_module,
  &http_module,
  &mpm_symbian_module,
#if defined(HAVE_MOD_SO_BUILTIN)
  &so_module,
#endif

#if defined(HAVE_MOD_ALIAS_BUILTIN)
  &alias_module,
#endif

#if defined(HAVE_MOD_ASIS_BUILTIN)
  &asis_module,
#endif

#if defined(HAVE_MOD_AUTH_BASIC_BUILTIN)
  &auth_basic_module,
#endif

#if defined(HAVE_MOD_AUTH_DIGEST_BUILTIN)
  &auth_digest_module,
#endif

#if defined(HAVE_MOD_AUTHN_ALIAS_BUILTIN)
  &authn_alias_module,
#endif

#if defined(HAVE_MOD_AUTHN_ANON_BUILTIN)
  &authn_anon_module,
#endif

#if defined(HAVE_MOD_AUTHN_DBD_BUILTIN)
  &authn_dbd_module,
#endif

#if defined(HAVE_MOD_AUTHN_DBM_BUILTIN)
  &authn_dbm_module,
#endif

#if defined(HAVE_MOD_AUTHN_DEFAULT_BUILTIN)
  &authn_default_module,
#endif

#if defined(HAVE_MOD_AUTHN_FILE_BUILTIN)
  &authn_file_module,
#endif

#if defined(HAVE_MOD_AUTHNZ_LDAP_BUILTIN)
  &authnz_ldap_module,
#endif

#if defined(HAVE_MOD_AUTHZ_DBM_BUILTIN)
  &authz_dbm_module,
#endif

#if defined(HAVE_MOD_AUTHZ_DEFAULT_BUILTIN)
  &authz_default_module,
#endif

#if defined(HAVE_MOD_AUTHZ_GROUPFILE_BUILTIN)
  &authz_groupfile_module,
#endif

#if defined(HAVE_MOD_AUTHZ_HOST_BUILTIN)
  &authz_host_module,
#endif

#if defined(HAVE_MOD_AUTHZ_USER_BUILTIN)
  &authz_user_module,
#endif

#if defined(HAVE_MOD_AUTOINDEX_BUILTIN)
  &autoindex_module,
#endif

#if defined(HAVE_MOD_BUCKETEER_BUILTIN)
  &bucketeer_module,
#endif

#if defined(HAVE_MOD_CACHE_BUILTIN)
  &cache_module,
#endif

#if defined(HAVE_MOD_CASE_FILTER_BUILTIN)
  &case_filter_module,
#endif

#if defined(HAVE_MOD_CASE_FILTER_IN_BUILTIN)
  &case_filter_in_module,
#endif

#if defined(HAVE_MOD_CERN_META_BUILTIN)
  &cern_meta_module,
#endif

#if defined(HAVE_MOD_CHARSET_LITE_BUILTIN)
  &charset_lite_module,
#endif

#if defined(HAVE_MOD_DAV_BUILTIN)
  &dav_module,
#endif

#if defined(HAVE_MOD_DAV_FS_BUILTIN)
  &dav_fs_module,
#endif

#if defined(HAVE_MOD_DAV_LOCK_BUILTIN)
  &dav_lock_module,
#endif

#if defined(HAVE_MOD_DEFLATE_BUILTIN)
  &deflate_module,
#endif

#if defined(HAVE_MOD_DIR_BUILTIN)
  &dir_module,
#endif

#if defined(HAVE_MOD_DUMPIO_BUILTIN)
  &dumpio_module,
#endif

#if defined(HAVE_MOD_ECHO_BUILTIN)
  &echo_module,
#endif

#if defined(HAVE_MOD_ENV_BUILTIN)
  &env_module,
#endif

#if defined(HAVE_MOD_EXAMPLE_BUILTIN)
  &exmaple_module,
#endif

#if defined(HAVE_MOD_EXPIRES_BUILTIN)
  &expires_module,
#endif

#if defined(HAVE_MOD_EXT_FILTER_BUILTIN)
  &ext_filter_module,
#endif

#if defined(HAVE_MOD_FILTER_BUILTIN)
  &filter_module,
#endif

#if defined(HAVE_MOD_HEADERS_BUILTIN)
  &headers_module,
#endif

#if defined(HAVE_MOD_IDENT_BUILTIN)
  &ident_module,
#endif

#if defined(HAVE_MOD_IMAGEMAP_BUILTIN)
  &imagemap_module,
#endif

#if defined(HAVE_MOD_INCLUDE_BUILTIN)
  &include_module,
#endif

#if defined(HAVE_MOD_INFO_BUILTIN)
  &info_module,
#endif

#if defined(HAVE_MOD_LOGIO_BUILTIN)
  &logio_module,
#endif

#if defined(HAVE_MOD_LOG_CONFIG_BUILTIN)
  &log_config_module,
#endif

#if defined(HAVE_MOD_LOG_FORENSIC_BUILTIN)
  &log_forensic_module,
#endif

#if defined(HAVE_MOD_MIME_BUILTIN)
  &mime_module,
#endif

#if defined(HAVE_MOD_MIME_MAGIC_BUILTIN)
  &mime_magic_module,
#endif

#if defined(HAVE_MOD_NEGOTIATION_BUILTIN)
  &negotiation_module,
#endif

#if defined(HAVE_MOD_PROXY_BUILTIN)
  &proxy_module,
#endif

#if defined(HAVE_MOD_PROXY_AJP_BUILTIN)
  &proxy_ajp_module,
#endif

#if defined(HAVE_MOD_PROXY_BALANCER_BUILTIN)
  &proxy_balancer_module,
#endif

#if defined(HAVE_MOD_PROXY_CONNECT_BUILTIN)
  &proxy_connect_module,
#endif

#if defined(HAVE_MOD_PROXY_FTP_BUILTIN)
  &proxy_ftp_module,
#endif

#if defined(HAVE_MOD_PROXY_HTTP_BUILTIN)
  &proxy_http_module,
#endif

#if defined(HAVE_MOD_REWRITE_BUILTIN)
  &rewrite_module,
#endif

#if defined(HAVE_MOD_SETENVIF_BUILTIN)
  &setenvif_module,
#endif

#if defined(HAVE_MOD_SPELING_BUILTIN)
  &speling_module,
#endif

#if defined(HAVE_MOD_SSL_BUILTIN)
  &ssl_module,
#endif

#if defined(HAVE_MOD_STATUS_BUILTIN)
  &status_module,
#endif

#if defined(HAVE_MOD_UNIQUE_ID_BUILTIN)
  &unique_id_module,
#endif

#if defined(HAVE_MOD_USERDIR_BUILTIN)
  &userdir_module,
#endif

#if defined(HAVE_MOD_USERTRACK_BUILTIN)
  &usertrack_module,
#endif

#if defined(HAVE_MOD_VERSION_BUILTIN)
  &version_module,
#endif

#if defined(HAVE_MOD_VHOST_ALIAS_BUILTIN)
  &vhost_alias_module,
#endif

#if defined(HAVE_MOD_SYMBIAN_BUILTIN)
  &symbian_calendar_module,
  &symbian_contacts_module,
  &symbian_dos_module,
  &symbian_logs_module,
  &symbian_messages_module,
  &symbian_proximity_module,
  &symbian_inbox_module,
#endif

#if defined(HAVE_MOD_S60_BUILTIN)
  &s60_camera_module,
  &s60_im_module,
  &s60_favourites_module,
#endif

  NULL
};
