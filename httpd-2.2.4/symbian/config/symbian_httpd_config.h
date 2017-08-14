#ifndef httpd_symbian_config_config_h
#define httpd_symbian_config_config_h
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

/* We now temporarily force the HTTPD root to be that used in MWS,
 * that is, "/data/web server". That way this version is backward
 * compatible with the current MWS release. Once MWS explicitly tells
 * what root it wants to use, we can remove this define alltogether.
 */

#include "symbian_httpd_builtin.h"


/***************************************************************************
 *
 * Platform Identifiers.
 *
 * These have been taken from the document 
 *
 *     S60 Platform:
 *     Identification Codes
 *     Version 2.0
 *     November 2, 2007
 *
 * Available at http://www.forum.nokia.com.
 */

#define PlatID_S60_3rd_Ed     0X101F7961
#define PlatID_S60_3rd_Ed_FP1 0x102032BE

/*
 * UID2 for static interface DLL.
 */
#define UID2_DLL 0x1000008d


/**************************************************************************/

#include "configure_symbian_httpd.h"


/***************************************************************************
 *
 * Here follows defaults.
 *
 * Do NOT edit these, but look into configure.h for how to change them
 * into something else.
 *
 **************************************************************************/


/***************************************************************************
 *
 * Platform UID.
 */
#if !defined(PlatID_S60_DEFAULT)
#define PlatID_S60_DEFAULT PlatID_S60_3rd_Ed
#endif


/*
 * Component UIDs.
 */
#if !defined(UID3_HTTPDS)
/* If the UID is ever changes, then change HTTPDS_BACKUP_TARGET below
 * as well.
 */
#define UID3_HTTPDS              0xe57b3de0
#else
#define UID3_HTTPDS_WAS_DEFINED
#endif

#if !defined(UID3_HTTPDUI)
#define UID3_HTTPDUI             0xe57b3de1
#endif


/* 
 * These are only used in installation files (.pkg).
 */
#if !defined(UID_PKG_HTTPD)
#define UID_PKG_HTTPD            0xe57b3de2
#endif

#if !defined(UID_PKG_HTDOCS)
#define UID_PKG_HTDOCS           0xe57b3de3
#endif

#if !defined(UID_PKG_HTTPDCONF)
#define UID_PKG_HTTPDCONF        0xe57b3de4
#endif

#if !defined(UID_PKG_HTTPDUTIL)
#define UID_PKG_HTTPDUTIL        0xe57b3de5
#endif

#if !defined(UID_PKG_HTTPDUI)
#define UID_PKG_HTTPDUI          0xe57b3de6
#endif


/***************************************************************************
 * 
 * Names
 */

#if !defined(HTTPDS_BACKUP_SOURCE)
#define HTTPDS_BACKUP_SOURCE "backup_registration.xml"
#endif

#if !defined(HTTPDS_BACKUP_TARGET)
#if defined(UID3_HTTPDS_WAS_DEFINED)
#error If you define UID3_HTTPDS, then you must define HTTPDS_BACKUP_TARGET
#else
#define HTTPDS_BACKUP_TARGET "!:\private\e57b3de0\backup_registration.xml"
#endif
#endif


/***************************************************************************
 *
 * Capabilities
 */

#if defined(MWS_WITH_DEVCERT)
#warning MWS_WITH_DEVCERT no longer carries any meaning.
#endif

#define MAX_OPEN_SIGNED_ONLINE_CAPABILITIES\
    LocalServices\
    NetworkServices\
    ReadUserData\
    WriteUserData\
    UserEnvironment\
    \
    Location\
    PowerMgmt\
    ProtServ\
    ReadDeviceData\
    SurroundingsDD\
    SwEvent\
    TrustedUI\
    WriteDeviceData                             


#if !defined(MWS_HTTPDS_CAPABILITIES)

#define MWS_HTTPDS_CAPABILITIES\
    LocalServices\
    NetworkServices\
    ReadUserData\
    WriteUserData\
    UserEnvironment\
    \
    Location

#endif /* MWS_HTTPDS_CAPABILITIES */


#if !defined(MWS_DLL_CAPABILITIES)

#define MWS_DLL_CAPABILITIES MWS_HTTPDS_CAPABILITIES

#endif /* MWS_DLL_CAPABILITIES */


#if !defined(MWS_HTTPDC_CAPABILITIES)

#define MWS_HTTPDC_CAPABILITIES MAX_OPEN_SIGNED_ONLINE_CAPABILITIES

#endif /* MWS_HTTPDC_CAPABILITIES */


#if !defined(MWS_HTTPDUTIL_CAPABILITIES)

#define MWS_HTTPDUTIL_CAPABILITIES MAX_OPEN_SIGNED_ONLINE_CAPABILITIES

#endif /* MWS_HTTPDUTIL_CAPABILITIES */



/***************************************************************************
 *
 * Components
 */

#if !defined(SIS_EXCLUDE_ALL)
#define SIS_INCLUDE_ALL
#endif

#if defined(SIS_INCLUDE_ALL)

#define SIS_INCLUDE_HTTPDS
#define SIS_INCLUDE_HTTPDC              

#define SIS_INCLUDE_LIBHTTPD            
#define SIS_INCLUDE_LIBAPR              
#define SIS_INCLUDE_LIBAPRUTIL          

#define SIS_INCLUDE_MOD_ACCESS          
#define SIS_INCLUDE_MOD_ALIAS           
#define SIS_INCLUDE_MOD_ASIS            
#define SIS_INCLUDE_MOD_AUTH            
#define SIS_INCLUDE_MOD_AUTHNZ_LDAP     
#define SIS_INCLUDE_MOD_AUTHN_ALIAS     
#define SIS_INCLUDE_MOD_AUTHN_ANON      
#define SIS_INCLUDE_MOD_AUTHN_DBD       
#define SIS_INCLUDE_MOD_AUTHN_DBM       
#define SIS_INCLUDE_MOD_AUTHN_DEFAULT   
#define SIS_INCLUDE_MOD_AUTHN_FILE      
#define SIS_INCLUDE_MOD_AUTHZ_DBM       
#define SIS_INCLUDE_MOD_AUTHZ_DEFAULT   
#define SIS_INCLUDE_MOD_AUTHZ_GROUPFILE 
#define SIS_INCLUDE_MOD_AUTHZ_HOST      
#define SIS_INCLUDE_MOD_AUTHZ_USER      
#define SIS_INCLUDE_MOD_AUTH_ANON       
#define SIS_INCLUDE_MOD_AUTH_BASIC      
#define SIS_INCLUDE_MOD_AUTH_DIGEST     
#define SIS_INCLUDE_MOD_AUTOINDEX       
#define SIS_INCLUDE_MOD_BUCKETEER       
#define SIS_INCLUDE_MOD_CACHE           
#define SIS_INCLUDE_MOD_CASE_FILTER     
#define SIS_INCLUDE_MOD_CASE_FILTER_IN  
#define SIS_INCLUDE_MOD_CERN_META       
#define SIS_INCLUDE_MOD_CHARSET_LITE    
#define SIS_INCLUDE_MOD_DAV             
#define SIS_INCLUDE_MOD_DAV_FS          
#define SIS_INCLUDE_MOD_DAV_LOCK        
#define SIS_INCLUDE_MOD_DEFLATE         
#define SIS_INCLUDE_MOD_DIR             
#define SIS_INCLUDE_MOD_DUMPIO          
#define SIS_INCLUDE_MOD_ECHO            
#define SIS_INCLUDE_MOD_ENV             
#define SIS_INCLUDE_MOD_EXAMPLE         
#define SIS_INCLUDE_MOD_EXPIRES         
#define SIS_INCLUDE_MOD_EXT_FILTER      
#define SIS_INCLUDE_MOD_FILTER          
#define SIS_INCLUDE_MOD_HEADERS         
#define SIS_INCLUDE_MOD_IDENT           
#define SIS_INCLUDE_MOD_IMAGEMAP        
#define SIS_INCLUDE_MOD_IMAP            
#define SIS_INCLUDE_MOD_INCLUDE         
#define SIS_INCLUDE_MOD_INFO            
#define SIS_INCLUDE_MOD_LOGIO           
#define SIS_INCLUDE_MOD_LOG_CONFIG      
#define SIS_INCLUDE_MOD_LOG_FORENSIC    
#define SIS_INCLUDE_MOD_MIME            
#define SIS_INCLUDE_MOD_MIME_MAGIC      
#define SIS_INCLUDE_MOD_NEGOTIATION     
#define SIS_INCLUDE_MOD_PROXY           
#define SIS_INCLUDE_MOD_PROXY_AJP       
#define SIS_INCLUDE_MOD_PROXY_BALANCER  
#define SIS_INCLUDE_MOD_PROXY_CONNECT   
#define SIS_INCLUDE_MOD_PROXY_FTP       
#define SIS_INCLUDE_MOD_PROXY_HTTP      
#define SIS_INCLUDE_MOD_REWRITE         
#define SIS_INCLUDE_MOD_SETENVIF        
#define SIS_INCLUDE_MOD_SPELING         
#define SIS_INCLUDE_MOD_SSL             
#define SIS_INCLUDE_MOD_STATUS          
#define SIS_INCLUDE_MOD_UNIQUE_ID       
#define SIS_INCLUDE_MOD_USERDIR         
#define SIS_INCLUDE_MOD_USERTRACK       
#define SIS_INCLUDE_MOD_VERSION         
#define SIS_INCLUDE_MOD_VHOST_ALIAS     

#define SIS_INCLUDE_MOD_S60             
#define SIS_INCLUDE_MOD_SYMBIAN         

#endif /* SIS_INCLUDE_ALL */


#if defined(SIS_EXCLUDE_HTTPDS)
#undef SIS_INCLUDE_HTTPDS
#endif

#if defined(SIS_EXCLUDE_HTTPDC)
#undef SIS_INCLUDE_HTTPDC  
#endif
           
#if defined(SIS_EXCLUDE_LIBHTTPD)
#undef SIS_INCLUDE_LIBHTTPD
#endif

#if defined(SIS_EXCLUDE_LIBAPR)
#undef SIS_INCLUDE_LIBAPR
#endif

#if defined(SIS_EXCLUDE_LIBAPRUTIL)
#undef SIS_INCLUDE_LIBAPRUTIL
#endif

#if defined(SIS_EXCLUDE_MOD_ACCESS)
#undef SIS_INCLUDE_MOD_ACCESS
#endif

#if defined(SIS_EXCLUDE_MOD_ALIAS)
#undef SIS_INCLUDE_MOD_ALIAS
#endif

#if defined(SIS_EXCLUDE_MOD_ASIS)
#undef SIS_INCLUDE_MOD_ASIS
#endif

#if defined(SIS_EXCLUDE_MOD_AUTH)
#undef SIS_INCLUDE_MOD_AUTH
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHNZ_LDAP)
#undef SIS_INCLUDE_MOD_AUTHNZ_LDAP
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHN_ALIAS)
#undef SIS_INCLUDE_MOD_AUTHN_ALIAS
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHN_ANON)
#undef SIS_INCLUDE_MOD_AUTHN_ANON
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHN_DBD)
#undef SIS_INCLUDE_MOD_AUTHN_DBD
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHN_DBM)
#undef SIS_INCLUDE_MOD_AUTHN_DBM
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHN_DEFAULT)
#undef SIS_INCLUDE_MOD_AUTHN_DEFAULT
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHN_FILE)
#undef SIS_INCLUDE_MOD_AUTHN_FILE
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHZ_DBM)
#undef SIS_INCLUDE_MOD_AUTHZ_DBM
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHZ_DEFAULT)
#undef SIS_INCLUDE_MOD_AUTHZ_DEFAULT
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHZ_GROUPFILE)
#undef SIS_INCLUDE_MOD_AUTHZ_GROUPFILE
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHZ_HOST)
#undef SIS_INCLUDE_MOD_AUTHZ_HOST
#endif

#if defined(SIS_EXCLUDE_MOD_AUTHZ_USER)
#undef SIS_INCLUDE_MOD_AUTHZ_USER
#endif

#if defined(SIS_EXCLUDE_MOD_AUTH_ANON)
#undef SIS_INCLUDE_MOD_AUTH_ANON
#endif

#if defined(SIS_EXCLUDE_MOD_AUTH_BASIC)
#undef SIS_INCLUDE_MOD_AUTH_BASIC
#endif

#if defined(SIS_EXCLUDE_MOD_AUTH_DIGEST)
#undef SIS_INCLUDE_MOD_AUTH_DIGEST
#endif

#if defined(SIS_EXCLUDE_MOD_AUTOINDEX)
#undef SIS_INCLUDE_MOD_AUTOINDEX
#endif

#if defined(SIS_EXCLUDE_MOD_BUCKETEER)
#undef SIS_INCLUDE_MOD_BUCKETEER
#endif

#if defined(SIS_EXCLUDE_MOD_CACHE)
#undef SIS_INCLUDE_MOD_CACHE
#endif

#if defined(SIS_EXCLUDE_MOD_CASE_FILTER)
#undef SIS_INCLUDE_MOD_CASE_FILTER
#endif

#if defined(SIS_EXCLUDE_MOD_CASE_FILTER_IN)
#undef SIS_INCLUDE_MOD_CASE_FILTER_IN
#endif

#if defined(SIS_EXCLUDE_MOD_CERN_META)
#undef SIS_INCLUDE_MOD_CERN_META
#endif

#if defined(SIS_EXCLUDE_MOD_CHARSET_LITE)
#undef SIS_INCLUDE_MOD_CHARSET_LITE
#endif

#if defined(SIS_EXCLUDE_MOD_DAV)
#undef SIS_INCLUDE_MOD_DAV
#endif

#if defined(SIS_EXCLUDE_MOD_DAV_FS)
#undef SIS_INCLUDE_MOD_DAV_FS
#endif

#if defined(SIS_EXCLUDE_MOD_DAV_LOCK)
#undef SIS_INCLUDE_MOD_DAV_LOCK
#endif

#if defined(SIS_EXCLUDE_MOD_DEFLATE)
#undef SIS_INCLUDE_MOD_DEFLATE
#endif

#if defined(SIS_EXCLUDE_MOD_DIR)
#undef SIS_INCLUDE_MOD_DIR
#endif

#if defined(SIS_EXCLUDE_MOD_DUMPIO)
#undef SIS_INCLUDE_MOD_DUMPIO
#endif

#if defined(SIS_EXCLUDE_MOD_ECHO)
#undef SIS_INCLUDE_MOD_ECHO
#endif

#if defined(SIS_EXCLUDE_MOD_ENV)
#undef SIS_INCLUDE_MOD_ENV
#endif

#if defined(SIS_EXCLUDE_MOD_EXAMPLE)
#undef SIS_INCLUDE_MOD_EXAMPLE
#endif

#if defined(SIS_EXCLUDE_MOD_EXPIRES)
#undef SIS_INCLUDE_MOD_EXPIRES
#endif

#if defined(SIS_EXCLUDE_MOD_EXT_FILTER)
#undef SIS_INCLUDE_MOD_EXT_FILTER
#endif

#if defined(SIS_EXCLUDE_MOD_FILTER)
#undef SIS_INCLUDE_MOD_FILTER
#endif

#if defined(SIS_EXCLUDE_MOD_HEADERS)
#undef SIS_INCLUDE_MOD_HEADERS
#endif

#if defined(SIS_EXCLUDE_MOD_IDENT)
#undef SIS_INCLUDE_MOD_IDENT
#endif

#if defined(SIS_EXCLUDE_MOD_IMAGEMAP)
#undef SIS_INCLUDE_MOD_IMAGEMAP
#endif

#if defined(SIS_EXCLUDE_MOD_IMAP)
#undef SIS_INCLUDE_MOD_IMAP
#endif

#if defined(SIS_EXCLUDE_MOD_INCLUDE)
#undef SIS_INCLUDE_MOD_INCLUDE
#endif

#if defined(SIS_EXCLUDE_MOD_INFO)
#undef SIS_INCLUDE_MOD_INFO
#endif

#if defined(SIS_EXCLUDE_MOD_LOGIO)
#undef SIS_INCLUDE_MOD_LOGIO
#endif

#if defined(SIS_EXCLUDE_MOD_LOG_CONFIG)
#undef SIS_INCLUDE_MOD_LOG_CONFIG
#endif

#if defined(SIS_EXCLUDE_MOD_LOG_FORENSIC)
#undef SIS_INCLUDE_MOD_LOG_FORENSIC
#endif

#if defined(SIS_EXCLUDE_MOD_MIME)
#undef SIS_INCLUDE_MOD_MIME
#endif

#if defined(SIS_EXCLUDE_MOD_MIME_MAGIC)
#undef SIS_INCLUDE_MOD_MIME_MAGIC
#endif

#if defined(SIS_EXCLUDE_MOD_NEGOTIATION)
#undef SIS_INCLUDE_MOD_NEGOTIATION
#endif

#if defined(SIS_EXCLUDE_MOD_PROXY)
#undef SIS_INCLUDE_MOD_PROXY
#endif

#if defined(SIS_EXCLUDE_MOD_PROXY_AJP)
#undef SIS_INCLUDE_MOD_PROXY_AJP
#endif

#if defined(SIS_EXCLUDE_MOD_PROXY_BALANCER)
#undef SIS_INCLUDE_MOD_PROXY_BALANCER
#endif

#if defined(SIS_EXCLUDE_MOD_PROXY_CONNECT)
#undef SIS_INCLUDE_MOD_PROXY_CONNECT
#endif

#if defined(SIS_EXCLUDE_MOD_PROXY_FTP)
#undef SIS_INCLUDE_MOD_PROXY_FTP
#endif

#if defined(SIS_EXCLUDE_MOD_PROXY_HTTP)
#undef SIS_INCLUDE_MOD_PROXY_HTTP
#endif

#if defined(SIS_EXCLUDE_MOD_REWRITE)
#undef SIS_INCLUDE_MOD_REWRITE
#endif

#if defined(SIS_EXCLUDE_MOD_SETENVIF)
#undef SIS_INCLUDE_MOD_SETENVIF
#endif

#if defined(SIS_EXCLUDE_MOD_SPELING)
#undef SIS_INCLUDE_MOD_SPELING
#endif

#if defined(SIS_EXCLUDE_MOD_SSL)
#undef SIS_INCLUDE_MOD_SSL
#endif

#if defined(SIS_EXCLUDE_MOD_STATUS)
#undef SIS_INCLUDE_MOD_STATUS
#endif

#if defined(SIS_EXCLUDE_MOD_UNIQUE_ID)
#undef SIS_INCLUDE_MOD_UNIQUE_ID
#endif

#if defined(SIS_EXCLUDE_MOD_USERDIR)
#undef SIS_INCLUDE_MOD_USERDIR
#endif

#if defined(SIS_EXCLUDE_MOD_USERTRACK)
#undef SIS_INCLUDE_MOD_USERTRACK
#endif

#if defined(SIS_EXCLUDE_MOD_VERSION)
#undef SIS_INCLUDE_MOD_VERSION
#endif

#if defined(SIS_EXCLUDE_MOD_VHOST_ALIAS)
#undef SIS_INCLUDE_MOD_VHOST_ALIAS
#endif

#if defined(SIS_EXCLUDE_MOD_S60)
#undef SIS_INCLUDE_MOD_S60
#endif

#if defined(SIS_EXCLUDE_MOD_SYMBIAN)
#undef SIS_INCLUDE_MOD_SYMBIAN
#endif

#endif
