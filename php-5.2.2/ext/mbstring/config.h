#ifndef __CONFIG_H__
#define __CONFIG_H__

#if defined( __SYMBIAN32__ ) && defined( __WINSCW__ )
#undef WIN
#undef WIN32
#undef _WIN
#undef _WIN32
#undef _WIN64
#undef __WIN__
#undef __WIN32__
#endif
#endif
