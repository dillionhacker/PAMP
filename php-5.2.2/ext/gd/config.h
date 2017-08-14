/* 

config.h

Manually created configuration file for 'ext\gd' for Symbian

*/

#ifndef __EXT_GD_CONFIG_H__

#include "../../symbian/config/openc.h"

#define HAVE_LIBJPEG 1

/* Referenced in: \ext\gd\libgd directory in files gd_png.c, gd_ss.c, gdparttopng.c, and gd2topng.c */
#define HAVE_LIBPNG 1

/* Start copy from 5.14. main/config.symbian.h */

/* Referenced in: \main\php_compat.h \ext\gd\php_gd.h \ext\gd\gd.c */
#define HAVE_GD_BUNDLED 1

/* Referenced in: \ext\gd\gd.c */
/* #undef HAVE_GD_FONTCACHESHUTDOWN
 */

/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_GD2 1

/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_PNG 1
#define HAVE_LIBPNG_PNG_H 1

/* Referenced in: \ext\gd\gd.c */
/* #undef HAVE_GD_STRINGTTF
 */

/* Referenced in: \ext\gd\php_gd.h \ext\gd\gd.c */
/* #undef HAVE_GD_STRINGFT
 */

/* Referenced in: \ext\gd\gd.c */
#define HAVE_LIBGD15 1

/* Referenced in: \ext\gd\gd.c \ext\gd\gdttf.c */
#define HAVE_LIBGD13 1 /* XXXsymbian: This may be really bad and access pixels in wrong order (x,y instead of y,x)*/
 
/* Referenced in: \ext\gd\php_gd.h \ext\gd\gd.c */
/* #define HAVE_GD_XPM 1 */
/* don't have  <X11/xpm.h> so don't have xpm */
#undef HAVE_GD_XPM

/* Referenced in: \ext\gd\gdcache.c */
/* #undef HAVE_GD_CACHE_CREATE
 */
/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_GIF_CTX 1

/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_IMAGESETBRUSH 1
 
/* Referenced in: \ext\gd\gd.c */
/* #undef HAVE_GD_STRINGFTEX
 */
/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_DYNAMIC_CTX_EX 1

/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_XBM 1

/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_IMAGEELLIPSE 1

/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_GIF_READ 1

/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_GIF_CREATE 1
 
/* Whether to build gd as dynamic module */
/* Referenced in: \ext\gd\gd.c */
/* #undef COMPILE_DL_GD
 */
/* Referenced in: \ext\gd\gd.c \ext\gd\gdttf.c */
/* #undef USE_GD_IMGSTRTTF
 */
/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_IMAGESETTILE 1

/* Referenced in: \ext\gd\php_gd.h \ext\gd\gd.c */
#define HAVE_LIBGD 1

/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_JPG 1
 
/* Referenced in: \ext\gd\gd.c \ext\gd\gdttf.c */
#define HAVE_GDIMAGECOLORRESOLVE 1

/* Referenced in: \ext\gd\gd.c */
/* #undef HAVE_GD_FREEFONTCACHE
 */
/* Referenced in: \ext\gd\gd.c */
/* #undef USE_GD_JISX0208
 */
/* Referenced in: \ext\gd\php_gd.h \ext\gd\gd.c \ext\gd\gdttf.c */
#define HAVE_LIBGD20 1
/* Referenced in: \ext\gd\gd.c */
#define HAVE_GD_WBMP 1
/* Referenced in: \ext\gd\gd_ctx.c \ext\gd\gd.c */
#define HAVE_LIBGD204 1


/* Referenced in: \ext\gd\gd.c */
/* #undef HAVE_LIBT1
 */
 
/* Define if you have the floorf function.  */
/* Have it in: \math.h */
/* Referenced in: \ext\gd\libgd\gd.c */
#undef HAVE_FLOORF

/* Define if you have the fabsf function.  */
/* Have it in: \math.h */
/* Referenced in: \ext\gd\libgd\gd.c */
//#define HAVE_FABSF 1
#undef HAVE_FABSF

/* Define if you have the <errno.h> header file.  */
/* yes */
/* Referenced in: \ext\gd\libgd\gdkanji.c */
#define HAVE_ERRNO_H 1  /* FIXsymbian: was undef */

/* Referenced in: \ext\gd\libgd\gdcache.c \ext\gd\gd.c \ext\gd\libgd\gdft.c \ext\gd\gdcache.c \ext\gd\php_gd.h \ext\gd\libgd\gdtestft.c */
#undef HAVE_LIBFREETYPE

/* Define if system uses EBCDIC */
/* Referenced in: \ext\gd\gd.c \ext\standard\url.c \ext\gd\gd_ctx.c \ext\gd\libgd\gd.c \ext\gd\gdttf.h \ext\gd\gdttf.c */
#undef CHARSET_EBCDIC

/* Referenced in: \ext\gd\libgd\gdcache.c \ext\gd\php_gd.h \ext\gd\gdcache.c \ext\gd\gd.c \ext\gd\gdttf.c */
#undef HAVE_LIBTTF

/* Referenced in: \ext\gd\gd.c */
#define HAVE_COLORCLOSESTHWB 1

// End copy from 5.14. main/config.symbian.h

#endif // __EXT_GD_CONFIG_H__
