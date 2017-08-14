/*
	PCRE config file for Symbian.
	
	Author(s): Petteri Muilu <petteri@futurice.com>
*/
#ifndef __SYMBIAN_PCRE_CONFIG__ 
#define __SYMBIAN_PCRE_CONFIG__ 

/*
	Include general open c config header.
*/
#include "../../symbian/config/openc.h"

/* PCRE keeps offsets in its compiled code as 2-byte quantities (always stored
in big-endian order) by default. These are used, for example, to link from the
start of a subpattern to its alternatives and its end. The use of 2 bytes per
offset limits the size of the compiled regex to around 64K, which is big enough
for almost everybody. However, I received a request for an even bigger limit.
For this reason, and also to make the code easier to maintain, the storing and
loading of offsets from the byte string is now handled by the macros that are
defined here.

The macros are controlled by the value of LINK_SIZE. This defaults to 2 in
the config.h file, but can be overridden by using -D on the command line. This
is automated on Unix systems via the "configure" command. */
#define LINK_SIZE 2

/*
 No idea what this is. NON-UNIX-USE states that this should be 10. So maybe it is it then. 
*/
#define POSIX_MALLOC_THRESHOLD 10

/* The value of NEWLINE determines the newline character. The default is to
leave it up to the compiler, but some sites want to force a particular value.
On Unix-like systems, "configure" can be used to override this default. */
#define NEWLINE '\n'

/* The value of MATCH_LIMIT determines the default number of times the internal
match() function can be called during a single execution of pcre_exec(). There
is a runtime interface for setting a different limit. The limit exists in order
to catch runaway regular expressions that take for ever to determine that they
do not match. The default is set very large so that it does not accidentally
catch legitimate cases. On systems that support it, "configure" can be used to
override this default default. */
#ifndef MATCH_LIMIT
#define MATCH_LIMIT 10000000
#endif

/* The above limit applies to all calls of match(), whether or not they
increase the recursion depth. In some environments it is desirable to limit the
depth of recursive calls of match() more strictly, in order to restrict the
maximum amount of stack (or heap, if NO_RECURSE is defined) that is used. The
value of MATCH_LIMIT_RECURSION applies only to recursive calls of match(). To
have any useful effect, it must be less than the value of MATCH_LIMIT. There is
a runtime method for setting a different limit. On systems that support it,
"configure" can be used to override this default default. */

#ifndef MATCH_LIMIT_RECURSION
#define MATCH_LIMIT_RECURSION MATCH_LIMIT
#endif

/* These three limits are parameterized just in case anybody ever wants to
change them. Care must be taken if they are increased, because they guard
against integer overflow caused by enormously large patterns. */

#ifndef MAX_NAME_SIZE
#define MAX_NAME_SIZE 32
#endif

#ifndef MAX_NAME_COUNT
#define MAX_NAME_COUNT 10000
#endif

#ifndef MAX_DUPLENGTH
#define MAX_DUPLENGTH 30000
#endif

#define SUPPORT_UTF8  1

#ifdef __ARMCC__
#pragma diag_remark 1134
#endif

#endif
