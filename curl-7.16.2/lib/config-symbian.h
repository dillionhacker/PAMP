/*
*	Config file for Symbian 
*
*/
#ifndef LIBCURL_CONFIG_SYMBIAN_H
#define LIBCURL_CONFIG_SYMBIAN_H


#include <sys/select.h>


#include "../symbian/config/openc.h"

/* Define if you have the recv function. */
#define HAVE_RECV

/* Define to the type of arg 1 for recv. */
#define RECV_TYPE_ARG1 int

/* Define to the type of arg 2 for recv. */
#define RECV_TYPE_ARG2 void *

/* Define to the type of arg 3 for recv. */
#define RECV_TYPE_ARG3 size_t

/* Define to the type of arg 4 for recv. */
#define RECV_TYPE_ARG4 int

/* Define to the function return type for recv. */
#define RECV_TYPE_RETV ssize_t

/* Define if you have the send function. */
#define HAVE_SEND 1

/* Define to the type of arg 1 for send. */
#define SEND_TYPE_ARG1 int

/* Define to the type qualifier of arg 2 for send. */
#define SEND_QUAL_ARG2 const

/* Define to the type of arg 2 for send. */
#define SEND_TYPE_ARG2 void *

/* Define to the type of arg 3 for send. */
#define SEND_TYPE_ARG3 int

/* Define to the type of arg 4 for send. */
#define SEND_TYPE_ARG4 size_t

/* Define to the function return type for send. */
#define SEND_TYPE_RETV int

/* Define this if you have struct timeval */
#define HAVE_STRUCT_TIMEVAL 1

#define HAVE_O_NONBLOCK

#define HAVE_SO_NONBLOCK

#define HAVE_ICONV

/**
 *  Disable LDAP for Symbian port. I don't think we need this?
 */ 
#define CURL_DISABLE_LDAP

#define OS "Symbian"

#endif
