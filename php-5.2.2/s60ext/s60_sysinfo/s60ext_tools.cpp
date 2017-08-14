#include <e32base.h>
#include <utf.h>
#include <HAL.h>
#include <bautils.h>

#include "s60ext_tools.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

TInt ZVAL_SYMBIAN_STRING(zval* output, const TDesC& input)
{
	if (input.Size() < 96) {
		TBuf8<256> utf8str;
		TInt error = CnvUtfConverter::ConvertFromUnicodeToUtf8(utf8str, input);
		if (error != KErrNone) {
			return error;
		}
		ZVAL_STRINGL(output, (char*)utf8str.Ptr(), utf8str.Size(), 1);
		return KErrNone;
	} else {
		TRAPD(error, {
			HBufC8* utf8str = HBufC8::NewLC(input.Length()*3);
			TPtr8 utf8ptr = utf8str->Des();
			TInt error = CnvUtfConverter::ConvertFromUnicodeToUtf8(utf8ptr, input);
			if (error != KErrNone) {
				User::Leave(error);
			}
			ZVAL_STRINGL(output, (char*)utf8str->Ptr(), utf8str->Size(), 1);
			CleanupStack::PopAndDestroy(utf8str);
		});
		return error;
	}
}


TInt convert_path_to_realpath(char* resolved_path, const char* path)
{
	TSRMLS_FETCH();

	//XXX relies on null termination of "path". Dangerous?
	//This is how PHP_FUNCTION(realpath) also does..
	if (tsrm_realpath(path, resolved_path TSRMLS_CC )) {
#ifdef ZTS
		if (VCWD_ACCESS(resolved_path, F_OK)) {
			return KErrPathNotFound;
		}
#endif
		return KErrNone;
	} else {
		return KErrPathNotFound;	
	}
}

TInt convert_c_path_to_unicode_realpath(TDes& uc, const char* string)
{
	char resolved_path_buff[MAXPATHLEN];
	TInt error = convert_path_to_realpath(resolved_path_buff, string);
	if (error != KErrNone) return error;
	
	return convert_c_string_to_unicode(uc, resolved_path_buff, strlen(resolved_path_buff));
}

TInt convert_c_string_to_unicode(TDes& uc, const char* string, int string_len) {
	return CnvUtfConverter::ConvertToUnicodeFromUtf8(uc, TPtr8((TUint8*)string, string_len, string_len));	
}


HBufC* convert_c_string_to_unicodeLC(const char* string, int string_len) {
	HBufC* pBuffer = HBufC::NewLC(string_len);
	TPtr16 p = pBuffer->Des();
	TInt error = convert_c_string_to_unicode(p, string, string_len);
	if (error) {
		User::Leave(KErrNoMemory);
	}
	return pBuffer;
}

/*
static TReal epoch_as_TReal()
{
  _LIT(KAppuifwEpoch, "19700000:");
  TTime epoch;
  epoch.Set(KAppuifwEpoch);
  return epoch.Int64().GetTReal();
}

TReal convert_time_to_utc_real(const TTime& aTime)
{
  TLocale loc;
  return (((aTime.Int64().GetTReal()-epoch_as_TReal())/
          (1000.0*1000.0))-
          TInt64(loc.UniversalTimeOffset().Int()).GetTReal());
}
*/
_LIT(KUnixEpoch, "19700000:");

TReal convert_time_to_utc_real(const TTime& time)
{
	// There will be roundoff errors here.
	#ifndef EKA2
		double unixtime=
			(time.Int64()-TTime(KUnixEpoch).Int64()).GetTReal();
	#else
		double unixtime= 
			TReal64(time.Int64()-TTime(KUnixEpoch).Int64());
	#endif
	unixtime /= 1e6;
	unixtime -= (TLocale().UniversalTimeOffset()).Int();
	#ifndef EKA2
	if (TLocale().QueryHomeHasDaylightSavingOn())
	{
		unixtime -= 3600;
	}	
	#endif
	return unixtime;
}

TTime convert_utc_real_to_time(TReal unixtime)
{
	#ifndef EKA2
	if (TLocale().QueryHomeHasDaylightSavingOn())
	{
		return TTime(TTime(KUnixEpoch).Int64()+
				(TInt64(unixtime)+TInt64(3600)+TInt64(TLocale().UniversalTimeOffset().Int()))*TInt64(1e6));
	}	
	#endif
	return TTime(TTime(KUnixEpoch).Int64()+
			(TInt64(unixtime)+TInt64(TLocale().UniversalTimeOffset().Int()))*TInt64(1e6));
}

TReal convert_utc_time_to_utc_real(const TTime& time)
{
	// There will be roundoff errors here.
	#ifndef EKA2
		double unixtime=
			(time.Int64()-TTime(KUnixEpoch).Int64()).GetTReal();
	#else
		double unixtime= 
			TReal64(time.Int64()-TTime(KUnixEpoch).Int64());
	#endif
	unixtime /= 1e6;
	return unixtime;
}

TTime convert_utc_real_to_utc_time(TReal unixtime)
{
	return TTime(TTime(KUnixEpoch).Int64()+
			(TInt64(unixtime))*TInt64(1e6));
}


extern "C" double
php_e32_clock()
{
  TInt period = 1000*1000;
  
  HAL::Get(HALData::ESystemTickPeriod, period);
  
  return (((double)(((double)User::TickCount())*
                    (((double)period)/1000.0)))/1000.0);
}

extern "C" double
php_e32_UTC_offset()
{
#ifndef EKA2
  TLocale loc;
  return TInt64(loc.UniversalTimeOffset().Int()).GetTReal();
#else
  TLocale loc;
  return TReal64(loc.UniversalTimeOffset().Int());
#endif /*EKA2*/
}



void set_symbian_error(TInt error) {
	switch (error) {
	// Some of the more typical errors produce a clear message
	case KErrNotFound:
		zend_error(E_WARNING, "Not found");
		break;
	case KErrGeneral:
		zend_error(E_WARNING, "General/Unknown error");
		break;
	//KErrCancel
	case KErrNoMemory:
		zend_error(E_WARNING, "Not enough memory");
		break;
	case KErrNotSupported:
		zend_error(E_WARNING, "Operation not supported");
		break;
	case KErrArgument:
		zend_error(E_WARNING, "Bad argument");
		break;
	//KErrTotaLossOfPrecision
	case KErrBadHandle:
		zend_error(E_WARNING, "Bad handle");
		break;
	case KErrOverflow:
		zend_error(E_WARNING, "Overflow");
		break;
	case KErrUnderflow:
		zend_error(E_WARNING, "Underflow");
		break;
	case KErrAlreadyExists:
		zend_error(E_WARNING, "Already exists");
		break;
	case KErrPathNotFound:
		zend_error(E_WARNING, "Unable to find the specified folder");
		break;
	// KErrDied
	case KErrInUse:
		zend_error(E_WARNING, "Object is in use elsewhere");
		break;
	// (-15) - (-20)
	case KErrAccessDenied:
		zend_error(E_WARNING, "Access denied");
		break;
	case KErrLocked:
		zend_error(E_WARNING, "Locked");
		break;
	case KErrWrite:
		zend_error(E_WARNING, "Couldn't write");
		break;
	// KErrDisMounted
	case KErrEof:
		zend_error(E_WARNING, "Unexpected end of file");
		break;
	case KErrDiskFull:
		zend_error(E_WARNING, "Disk full");
		break;
	// (-27) - (-45)
	case KErrPermissionDenied:
		zend_error(E_WARNING, "Permission denied");
		break;
	// (-47) -
	case KErrPhpCustom:
		// zend_error has already been called when custom error occurs
		break;
	// The rest will just show the error code
	default:
		zend_error(E_WARNING, "Unknown symbian error: %d", error);
	}	
}

TBool doesFileExistL(TDesC& filename)
{
	TBool fileExists = EFalse;
	RFs fileSession;
	User::LeaveIfError(fileSession.Connect());
	CleanupClosePushL(fileSession);
	fileExists = BaflUtils::FileExists(fileSession, filename);
	CleanupStack::PopAndDestroy(); // Close fileSession.
	return fileExists;
}

