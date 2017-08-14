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
 * Note: This is a Symbian specific version of apu_config.hs. It is copied
 * as apu_config.h at the start of a Symbian build.
 */

#ifdef __SYMBIAN32__

#ifndef APU_CONFIG_H
#define APU_CONFIG_H

/* define if Expat 1.0 or 1.1 was found */
#undef APR_HAVE_OLD_EXPAT

/* Define if the system crypt() function is threadsafe */
#undef APU_CRYPT_THREADSAFE

/* Define if the inbuf parm to iconv() is const char ** */
#undef APU_ICONV_INBUF_CONST

/* Define if crypt_r has uses CRYPTD */
#undef CRYPT_R_CRYPTD

/* Define if crypt_r uses struct crypt_data */
#undef CRYPT_R_STRUCT_CRYPT_DATA

/* Define if CODESET is defined in langinfo.h */
#undef HAVE_CODESET

/* Define to 1 if you have the `crypt_r' function. */
#undef HAVE_CRYPT_R

/* Define to 1 if you have the file `AC_File'. */
#undef HAVE_DBD_APR_DBD_MYSQL_C

/* Define to 1 if you have the <iconv.h> header file. */
#define HAVE_ICONV_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <langinfo.h> header file. */
#define HAVE_LANGINFO_H 1

/* Define to 1 if you have the <lber.h> header file. */
#undef HAVE_LBER_H

/* Defined if ldap.h is present */
#undef HAVE_LDAP_H

/* Define to 1 if you have the <ldap_ssl.h> header file. */
#undef HAVE_LDAP_SSL_H

/* Define to 1 if you have the `lber' library (-llber). */
#undef HAVE_LIBLBER

/* Define to 1 if you have the <libpq-fe.h> header file. */
#undef HAVE_LIBPQ_FE_H

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <mysql.h> header file. */
#undef HAVE_MYSQL_H

/* Define to 1 if you have the <mysql/mysql.h> header file. */
#undef HAVE_MYSQL_MYSQL_H

/* Define to 1 if you have the `nl_langinfo' function. */
#undef HAVE_NL_LANGINFO

/* Define to 1 if you have the <postgresql/libpq-fe.h> header file. */
#undef HAVE_POSTGRESQL_LIBPQ_FE_H

/* Define to 1 if you have the <sqlite3.h> header file. */
#undef HAVE_SQLITE3_H

/* Define to 1 if you have the <sqlite.h> header file. */
#undef HAVE_SQLITE_H

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS


#endif /* APU_CONFIG_H */
#endif /* __SYMBIAN32__ */
