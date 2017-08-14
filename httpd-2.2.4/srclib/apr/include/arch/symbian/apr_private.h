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

/*
 * Note: 
 * This is the Symbian specific autoconf-like config file
 * which unix would create at build time.
 */

#ifdef __SYMBIAN32__

#ifndef APR_PRIVATE_H
#define APR_PRIVATE_H

/*
 * See ../unix/apr_private.h.in for an explanation for the defines.
 */

#define APR_INT64_STRFN                strtoll
#define APR_OFF_T_STRFN                strtol
#undef  DEV_RANDOM
#undef  DIRENT_INODE
#define DIRENT_TYPE                    d_type
#define DSO_USE_DLFCN                  1
#undef  DSO_USE_DYLD
#undef  DSO_USE_SHL
#undef  EGD_DEFAULT_SOCKET
#undef  FCNTL_IS_GLOBAL
#undef  FLOCK_IS_GLOBAL
// FIXME: What is the situation actually? JWi
#undef  GETHOSTBYADDR_IS_THREAD_SAFE
// FIXME: What is the situation actually? JWi
#undef  GETHOSTBYNAME_IS_THREAD_SAFE
#undef  GETHOSTBYNAME_R_GLIBC2
#undef  GETHOSTBYNAME_R_HOSTENT_DATA
#undef  HAVE_ALLOCA
#undef  HAVE_ALLOCA_H
#define HAVE_ARPA_INET_H               1
#undef  HAVE_BONE_VERSION
#undef  HAVE_BYTEORDER_H
#define HAVE_CALLOC                    1
#undef  HAVE_CONIO_H
#undef  HAVE_CREATE_AREA
#undef  HAVE_CREATE_SEM
#undef  HAVE_CRYPT_H
#define HAVE_CTYPE_H                   1
#undef  HAVE_DECL_SYS_SIGLIST
#define HAVE_DIRENT_H                  1
#undef  HAVE_DIR_H
#define HAVE_DLFCN_H                   1
#undef  HAVE_DL_H
#undef  HAVE_EGD
#undef  HAVE_EPOLL
#define HAVE_ERRNO_H                   1
#define HAVE_FCNTL_H                   1 
#undef  HAVE_FLOCK       
#undef  HAVE_FORK            
#define HAVE_F_SETLK                   1 
// FIXME: OpenC does have getaddrinfo, but something is not working right
// FIXME: since the return value is always EAI_BADFLAGS. For the time
// FIXME: being we just pretend the functions are not there.
//#define HAVE_GAI_ADDRCONFIG            1
//#define HAVE_GAI_STRERROR              1
//#define HAVE_GETADDRINFO               1
#undef  HAVE_GAI_ADDRCONFIG
#undef  HAVE_GAI_STRERROR
#undef  HAVE_GETADDRINFO
#define HAVE_GETENV                    1
#define HAVE_GETGRGID_R                1
#define HAVE_GETGRNAM_R                1
#undef  HAVE_GETHOSTBYADDR_R           
#undef  HAVE_GETHOSTBYNAME_R
#undef  HAVE_GETIFADDRS
#define HAVE_GETNAMEINFO               1
#undef  HAVE_GETPASS                   
#undef  HAVE_GETPASSPHRASE
#define HAVE_GETPWNAM_R                1
#define HAVE_GETPWUID_R                1
#undef  HAVE_GETRLIMIT
#undef  HAVE_GMTIME_R
#define HAVE_GRP_H                     1
#undef  HAVE_HSTRERROR
#define HAVE_INTTYPES_H                1
#undef  HAVE_IO_H
#define HAVE_ISINF                     1
#define HAVE_ISNAN
#undef  HAVE_KERNEL_OS_H
#undef  HAVE_KQUEUE
#define HAVE_LANGINFO_H                1
#undef  HAVE_LIBBSD
#undef  HAVE_LIBSENDFILE
#undef  HAVE_LIBTRUERAND
#define HAVE_LIMITS_H                  1
#define HAVE_LOCALTIME_R               1
#undef  HAVE_LOCK_EX
#undef  HAVE_MACH_O_DYLD_H
#undef  HAVE_MALLOC_H
#define HAVE_MAP_ANON                  1
#define HAVE_MEMCHR                    1
#define HAVE_MEMMOVE                   1
#define HAVE_MEMORY_H                  1
#define HAVE_MKSTEMP                   1
#undef  HAVE_MKSTEMP64
#define HAVE_MMAP                      1
#undef  HAVE_MMAP64
#define HAVE_MUNMAP                    1
#define HAVE_NETDB_H                   1
#define HAVE_NETINET_IN_H              1
#undef  HAVE_NETINET_SCTP_H
#undef  HAVE_NETINET_SCTP_UIO_H
#undef  HAVE_NETINET_TCP_H
#undef  HAVE_NET_ERRNO_H
#define HAVE_NL_LANGINFO
#undef  HAVE_OS2_H
#undef  HAVE_OSRELDATE_H
#undef  HAVE_OS_H
#undef  HAVE_POLL
#undef  HAVE_POLLIN
#undef  HAVE_POLL_H
#undef  HAVE_PORT_CREATE
#undef  HAVE_PROCESS_H
#undef  HAVE_PTHREAD_ATTR_SETGUARDSIZE
#define HAVE_PTHREAD_H                    1
#define HAVE_PTHREAD_KEY_DELETE           1
#define HAVE_PTHREAD_MUTEXATTR_SETPSHARED 1
#define HAVE_PTHREAD_MUTEX_RECURSIVE      1
#undef  HAVE_PTHREAD_MUTEX_ROBUST
#undef  HAVE_PTHREAD_PROCESS_SHARED
#undef  HAVE_PTHREAD_RWLOCKS
#undef  HAVE_PTHREAD_RWLOCK_INIT
#define HAVE_PUTENV                       1 
#define HAVE_PWD_H                        1
#define HAVE_SEMAPHORE_H                  1
#define HAVE_SEMCTL                       1
#define HAVE_SEMGET                       1
#define HAVE_SEM_CLOSE                    1
#define HAVE_SEM_POST                     1
#undef  HAVE_SEM_UNDO
#define HAVE_SEM_UNLINK                   1
#define HAVE_SEM_WAIT                     1
#undef  HAVE_SENDFILE
#undef  HAVE_SENDFILE64
#undef  HAVE_SENDFILEV
#undef  HAVE_SENDFILEV64
#undef  HAVE_SEND_FILE
#define HAVE_SETENV                       1
#undef  HAVE_SETRLIMIT
#define HAVE_SETSID                       1
#undef  HAVE_SET_H_ERRNO
#define HAVE_SHMAT                        1
#define HAVE_SHMCTL                       1
#define HAVE_SHMDT                        1
#define HAVE_SHMGET                       1
#undef  HAVE_SHM_OPEN
#undef  HAVE_SHM_UNLINK
#define HAVE_SIGACTION                    1
#define HAVE_SIGNAL_H                     1
#undef  HAVE_SIGSUSPEND
#undef  HAVE_SIGWAIT
#define HAVE_SOCKLEN_T                    1
#define HAVE_SO_ACCEPTFILTER              1
#define HAVE_STDARG_H                     1
#define HAVE_STDDEF_H                     1
#define HAVE_STDINT_H                     1
#define HAVE_STDIO_H                      1
#define HAVE_STDLIB_H                     1
#define HAVE_STRCASECMP                   1
#define HAVE_STRDUP                       1
#define HAVE_STRERROR_R                   1
#undef  HAVE_STRICMP
#define HAVE_STRINGS_H                    1
#define HAVE_STRING_H                     1
#define HAVE_STRNCASECMP                  1
#undef  HAVE_STRNICMP
#define HAVE_STRSTR                       1
#undef  HAVE_STRUCT_IPMREQ
#define HAVE_STRUCT_TM_TM_GMTOFF          1 
#undef  HAVE_STRUCT_TM___TM_GMTOFF
#undef  HAVE_SYSAPI_H
#undef  HAVE_SYSGTIME_H
#undef  HAVE_SYS_FILE_H
#undef  HAVE_SYS_IOCTL_H
#define HAVE_SYS_IPC_H                    1
#define HAVE_SYS_MMAN_H                   1
#undef  HAVE_SYS_MUTEX_H
// sys/poll.h exists, but poll is not defined, only declared.
#undef  HAVE_SYS_POLL_H
#define HAVE_SYS_RESOURCE_H               1
#define HAVE_SYS_SELECT_H                 1
#define HAVE_SYS_SEM_H                    1
#undef  HAVE_SYS_SENDFILE_H
#define HAVE_SYS_SHM_H                    1
#define HAVE_SYS_SIGNAL_H                 1
#define HAVE_SYS_SOCKET_H                 1 
#define HAVE_SYS_SOCKIO_H                 1
#define HAVE_SYS_STAT_H                   1
#define HAVE_SYS_SYSCTL_H                 1
#define HAVE_SYS_SYSLIMITS_H              1
#define HAVE_SYS_TIME_H                   1
#define HAVE_SYS_TYPES_H                  1
#define HAVE_SYS_UIO_H                    1
#define HAVE_SYS_UN_H                     1
#undef  HAVE_SYS_UUID_H
#define HAVE_SYS_WAIT_H                   1
#undef  HAVE_TCP_CORK
#undef  HAVE_TCP_NODELAY_WITH_CORK
#undef  HAVE_TCP_NOPUSH
#undef  HAVE_TERMIOS_H
#define HAVE_TIME_H                       1
#undef  HAVE_TPFEQ_H
#undef  HAVE_TPFIO_H
#undef  HAVE_TRUERAND
#define HAVE_UNISTD_H                     1
#undef  HAVE_UNIX_H
#define HAVE_UNSETENV                     1 
#define HAVE_UTIME                        1 
#define HAVE_UTIMES                       1
#undef  HAVE_UUID_CREATE
#undef  HAVE_UUID_GENERATE
#undef  HAVE_UUID_H
#undef  HAVE_UUID_UUID_H
#define HAVE_VLA                          1
#define HAVE_WAITPID                      1
#define HAVE_WRITEV                       1

// FIXME: What's the case? JWi
#undef NEGATIVE_EAI

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

/* Define if POSIX semaphores affect threads within the process */
#undef POSIXSEM_IS_GLOBAL

#undef PTHREAD_ATTR_GETDETACHSTATE_TAKES_ONE_ARG
#undef PTHREAD_GETSPECIFIC_TAKES_TWO_ARGS

/* Define if readdir is thread safe */
#define READDIR_IS_THREAD_SAFE 1

#undef SETPGRP_VOID
#undef SIGWAIT_TAKES_ONE_ARG

#define SIZEOF_CHAR      1
#define SIZEOF_INT       2
#define SIZEOF_LONG      4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_OFF_T     4
#define SIZEOF_PID_T     4
#define SIZEOF_SHORT     2
#define SIZEOF_SIZE_T    4
#define SIZEOF_SSIZE_T   4
#define SIZEOF_VOIDP     4

#define STDC_HEADERS                      1

#define STRERROR_R_RC_INT                 1

// FIXME: do we have these? JWi
#undef SYSVSEM_IS_GLOBAL

#undef USE_BEOSSEM

/* Define if SVR4-style fcntl() will be used */
#undef USE_FCNTL_SERIALIZE

/* Define if 4.2BSD-style flock() will be used */
#undef USE_FLOCK_SERIALIZE

/* Define if use of generic atomics is requested */
#undef USE_GENERIC_ATOMICS

/* Define if BeOS areas will be used */
#undef USE_SHMEM_BEOS

/* Define if BeOS areas will be used */
#undef USE_SHMEM_BEOS_ANON

/* Define if 4.4BSD-style mmap() via MAP_ANON will be used */
#undef USE_SHMEM_MMAP_ANON

/* Define if mmap() via POSIX.1 shm_open() on temporary file will be used */
#undef USE_SHMEM_MMAP_SHM

/* Define if Classical mmap() on temporary file will be used */
#undef USE_SHMEM_MMAP_TMP

/* Define if SVR4-style mmap() on /dev/zero will be used */
#undef USE_SHMEM_MMAP_ZERO

/* Define if OS/2 DosAllocSharedMem() will be used */
#undef USE_SHMEM_OS2

/* Define if OS/2 DosAllocSharedMem() will be used */
#undef USE_SHMEM_OS2_ANON

/* Define if SysV IPC shmget() will be used */
#undef USE_SHMEM_SHMGET

/* Define if SysV IPC shmget() will be used */
#undef USE_SHMEM_SHMGET_ANON

/* Define if SysV IPC semget() will be used */
#undef USE_SYSVSEM_SERIALIZE

/* Define if apr_wait_for_io_or_timeout() uses poll(2) */
#undef WAITIO_USES_POLL

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#undef WORDS_BIGENDIAN

/* Define to 1 if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
# undef _ALL_SOURCE
#endif

/* Define to 1 if on MINIX. */
#undef _MINIX

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#undef _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
#undef _POSIX_SOURCE

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Define to `int' if <sys/types.h> doesn't define. */
#undef gid_t

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif

/* Define to `long' if <sys/types.h> does not define. */
#undef off_t

/* Define to `int' if <sys/types.h> does not define. */
#undef pid_t

/* Define to `unsigned' if <sys/types.h> does not define. */
#undef size_t

/* Define to `int' if <sys/types.h> does not define. */
#undef ssize_t

/* Define to `int' if <sys/types.h> doesn't define. */
#undef uid_t


/* switch this on if we have a BeOS version below BONE */
#if BEOS && !HAVE_BONE_VERSION
#define BEOS_R5 1
#else
#define BEOS_BONE 1
#endif

/*
 * Include common private declarations.
 */
#include "../apr_private_common.h"

#endif /* APR_PRIVATE_H */
#endif /* __SYMBIAN32__ */
