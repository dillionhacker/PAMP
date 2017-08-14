/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2007 Nokia Corporation                              	  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
 */

#include "TSRM.h"

#ifdef __SYMBIAN32__

extern "C" {
#include "tsrm_symbian.h"
}
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
extern "C" {
#include "tsrm_virtual_cwd.h"
#include "php.h"
}
#include <f32file.h>
#include <e32err.h>
#include <e32cmn.h>

extern "C" {

static int map_symbian_error_to_errno(const int error)
{
	switch (error) {
		case KErrNotFound:
		case KErrPathNotFound:
			return ENOENT;
		case KErrCancel:
			return EINTR;
		case KErrNoMemory:
			return ENOMEM;
		case KErrNotSupported:
			return ENOSYS;
		case KErrArgument:
		case KErrBadDescriptor:
		case KErrBadName:
			return EINVAL;
		case KErrTotalLossOfPrecision:
			return ERANGE;
		case KErrBadHandle:
			return EBADF;
		case KErrOverflow:
		case KErrUnderflow:
			return ERANGE;
		case KErrAlreadyExists:
			return EEXIST;
		case KErrDied:
		case KErrDisconnected:
		case KErrSessionClosed:
		case KErrEof:
		case KErrServerTerminated:
			return EPIPE;
		case KErrInUse:
		case KErrPermissionDenied:
		case KErrAccessDenied:
		case KErrLocked:
			return EACCES;
		case KErrServerBusy:
			return EBUSY;
		case KErrUnknown:
		case KErrHardwareNotAvailable:
		case KErrDisMounted:
			return ENODEV;
		case KErrWrite:
			return EIO;
		case KErrDiskFull:
		case KErrDirFull:
			return ENOSPC;
		case KErrCommsLineFail:
		case KErrCommsFrame:
		case KErrCommsOverrun:
		case KErrCommsParity:
		case KErrCommsBreak:
			return ECOMM;
		case KErrTimedOut:
			return ETIMEDOUT;
		case KErrCouldNotConnect:
			return ECONNREFUSED;
		case KErrTooBig:
			return EFAULT;
		case KErrDivideByZero:
		case KErrBadPower:
			return EDOM;
		default:
			return ENODEV; // because KErrUnknown maps to this too.
	}
}

static void normalize_path(char* path)
{
	int i;
	for( i = 0; i < MAXPATHLEN && path[i] != '\0'; ++i) {
		if( IS_SLASH(path[i]) ){
			path[i] = DEFAULT_SLASH;
		}
	}
}

TSRM_API int tsrm_symbian_unlink(const char *path)
{
	int err;
	RFs fs;
	char normalized_path[MAXPATHLEN];

	strncpy(normalized_path, path, MAXPATHLEN);
	normalize_path(normalized_path);
	path = normalized_path;

	if( err = fs.Connect() == KErrNone )
	{
		TBuf8<255> fname8( reinterpret_cast<const TUint8*>( path ) );
		
		TBuf<255> fname16;
		
		if( fname8.Length() > 0 && fname8[0] == '.' )
		{
			TBuf<255> fname16tmp;
			fname16tmp.Copy( fname8 );
			
			char* buf = (char*)malloc( 255 );
			char* ptr = getcwd( buf, 255 );
			
			TBuf8<255> wd( reinterpret_cast<const TUint8*>( ptr ) );
			
			free( buf );
			
			fname16.Copy( wd );
			fname16.Append( fname16tmp.Right( fname8.Length() - 1 ) );
		}
		else {
			fname16.Copy( fname8 );
		}
		err = fs.Delete( fname16 );
		fs.Close();
		if( err == KErrNone )	{
			return 0;
		} else {
			/* FIXME Error handling, can we set errno? */
			errno = map_symbian_error_to_errno(err);
			return -1;
		}
		
	} else {
		/* FIXME Error handling, can we set errno? */
		errno = map_symbian_error_to_errno(err);
		return -1;
	}
} // tsrm_symbian_unlink

} // extern "C"

#endif
