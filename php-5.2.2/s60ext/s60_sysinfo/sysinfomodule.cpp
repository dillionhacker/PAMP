/**
 * ====================================================================
 *  PHP API to system information, modified from corresponding
 *  Python API. The module consists of a bunch of inspection functions.
 * --------------------------------------------------------------------
 * 
 * Copyright (c) 2006 Nokia Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ====================================================================
 */
 
/**
//EXAMPLE

<pre><?php

//Test script that prints out all the information:

echo 'os_version(): ';    var_dump(s60_sysinfo_os_version());
echo 'sw_version(): ';    var_dump(s60_sysinfo_sw_version());
echo 'sw_platform(): ';   var_dump(s60_sysinfo_sw_platform());
echo 'device_model(): ';  var_dump(s60_sysinfo_device_model());

echo 'ram_total(): ';     var_dump(s60_sysinfo_ram_total());
echo 'ram_free(): ';      var_dump(s60_sysinfo_ram_free());
echo 'rom_total(): ';     var_dump(s60_sysinfo_rom_total());

echo 'drive_free(): ';    var_dump(s60_sysinfo_drive_free());
echo 'display_twips(): '; var_dump(s60_sysinfo_display_twips());
echo 'display_pixels(): ';var_dump(s60_sysinfo_display_pixels());
echo 'battery(): ';       var_dump(s60_sysinfo_battery());
echo 'battery_max(): ';   var_dump(s60_sysinfo_battery_max());
echo 'signal(): ';        var_dump(s60_sysinfo_signal());
echo 'signal_max(): ';    var_dump(s60_sysinfo_signal_max());
echo 'imei(): ';          var_dump(s60_sysinfo_imei());
echo 'in_emulator(): ';   var_dump(s60_sysinfo_in_emulator());

?></pre>
 
*/

 
#include <eikenv.h>
#include <e32std.h>
#include <sysutil.h>	// OS, SW info
#include <hal.h>		// HAL info
#include <centralrepository.h>
#include <ProfileEngineSDKCRKeys.h>

#include "s60ext_tools.h"

#include <avkon.rsg>
#include <aknglobalmsgquery.h> 


#ifndef EKA2
#include <plpvariant.h> // IMEI
#include <saclient.h>   // Battery, network, see also sacls.h
#else /*EKA2*/
//#include <centralrepository.h>
//#include <ProfileEngineSDKCRKeys.h>
//#include <Etel3rdParty.h>
#include "asynccallhandler.h"
#endif /*EKA2*/
//#include <f32file.h> 

namespace {

const TInt KPhoneSwVersionLineFeed = '\n';

#ifndef EKA2
// UID for network signal strength
const TInt KUidNetworkBarsValue = 0x100052D4;
const TUid KUidNetworkBars = {KUidNetworkBarsValue};

// UID for battery level
const TInt KUidBatteryBarsValue = 0x100052D3;
const TUid KUidBatteryBars = {KUidBatteryBarsValue};
#endif /*EKA2*/

}


extern "C" {
	
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h" 
 	
#include "sysinfomodule.h"

BEGIN_MODULE_FUNCTIONS(s60_sysinfo)
	PHP_FE(s60_sysinfo_os_version, NULL)
	PHP_FE(s60_sysinfo_sw_version, NULL)
	PHP_FE(s60_sysinfo_sw_platform, NULL)
	PHP_FE(s60_sysinfo_device_model, NULL)
	
	PHP_FE(s60_sysinfo_ram_total, NULL)
	PHP_FE(s60_sysinfo_ram_free, NULL)
	PHP_FE(s60_sysinfo_rom_total, NULL)
	
	PHP_FE(s60_sysinfo_drive_free, NULL)
	PHP_FE(s60_sysinfo_display_twips, NULL)
	PHP_FE(s60_sysinfo_display_pixels, NULL)
	PHP_FE(s60_sysinfo_battery, NULL)
	PHP_FE(s60_sysinfo_battery_max, NULL)
	PHP_FE(s60_sysinfo_signal, NULL)
	PHP_FE(s60_sysinfo_signal_max, NULL)
   	PHP_FE(s60_sysinfo_imei, NULL)
   	PHP_FE(s60_sysinfo_in_emulator, NULL)
   	PHP_FE(s60_sysinfo_current_profile, NULL)
   	PHP_FE(s60_message_query, NULL)
   	
END_MODULE_FUNCTIONS()

DEFINE_MODULE(s60_sysinfo, "1.0", NULL, NULL, NULL, NULL)


/**
 * <array> s60_sysinfo_os_version()
 * Returns the operating system version.
 */		
PHP_FUNCTION(s60_sysinfo_os_version)
{
	GET_FUNCTION_ARGS_0();
	TVersion version;
	version = User::Version();
	
	array_init(return_value);
	add_assoc_long(return_value, "major", version.iMajor);
	add_assoc_long(return_value, "minor", version.iMinor);
	add_assoc_long(return_value, "build", version.iBuild);
}


/**
 * <string> s60_sysinfo_sw_version()
 * Returns the software version of the device.
 */
PHP_FUNCTION(s60_sysinfo_sw_version)
{
	GET_FUNCTION_ARGS_0();
#ifdef S60_EMULATOR
	RETURN_STRING("emulator", 1);
#else
	TBufC<KSysUtilVersionTextLength> version;
	TPtr ptr(version.Des());
	TInt error = KErrNone;
	
	error = SysUtil::GetSWVersion(ptr);
	RETURN_IF_SYMBIAN_ERROR(error);

	TInt index = 0;

	for (; index < ptr.Length(); index++)
		{
		if (ptr[index] == KPhoneSwVersionLineFeed)
			{
			ptr[index] = ' ';
			}
		}

	error = ZVAL_SYMBIAN_STRING(return_value, version);
	RETURN_IF_SYMBIAN_ERROR(error);
#endif
}


/**
 * <string> s60_sysinfo_sw_platform()
 * Returns the software platform of the device.
 * On S60 version 2.x, returns "EKA1".
 * On S60 version 3.x, returns "EKA2".
 */
PHP_FUNCTION(s60_sysinfo_sw_platform)
{
	GET_FUNCTION_ARGS_0();
#ifdef EKA2
	RETURN_STRING("EKA2", 1);
#else
	RETURN_STRING("EKA1", 1);
#endif
}

/**
 * <int> s60_sysinfo_device_model()
 * Returns the device model.
 */
PHP_FUNCTION(s60_sysinfo_device_model)
{
	GET_FUNCTION_ARGS_0();
	TInt eValue;
	TInt error = HAL::Get(HALData::EModel, eValue);
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_LONG(eValue);
}


/**
 * <int> s60_sysinfo_ram_total()
 * Returns the total amount of RAM in the device.
 */
PHP_FUNCTION(s60_sysinfo_ram_total)
{
	GET_FUNCTION_ARGS_0();
	TInt eValue;
	TInt error = HAL::Get(HALData::EMemoryRAM, eValue);
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_LONG(eValue);
}

/**
 * <int> s60_sysinfo_ram_free()
 * Returns the free ram available.
 */
PHP_FUNCTION(s60_sysinfo_ram_free)
{
	GET_FUNCTION_ARGS_0();
	TInt eValue;
	TInt error = HAL::Get(HALData::EMemoryRAMFree, eValue);
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_LONG(eValue);
}

/**
 * <int> s60_sysinfo_rom_total()
 * Returns the total amount of ROM in the device.
 */
PHP_FUNCTION(s60_sysinfo_rom_total)
{
	GET_FUNCTION_ARGS_0();
	TInt eValue;
	TInt error = HAL::Get(HALData::EMemoryROM, eValue);
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_LONG(eValue);
}

/**
 * <array> s60_sysinfo_drive_free()
 * Returns the drives and the amount of free space in the device.
 */
PHP_FUNCTION(s60_sysinfo_drive_free)
{
	GET_FUNCTION_ARGS_0();
	TInt error = KErrNone;
	RFs fsSession;
	error = fsSession.Connect();
	RETURN_IF_SYMBIAN_ERROR(error);
	
	array_init(return_value);

	TVolumeInfo volumeInfo; 
	TInt driveNumber;
	
	for (driveNumber = EDriveA; driveNumber <= EDriveZ; driveNumber++) {
		TInt err = fsSession.Volume(volumeInfo,driveNumber); 
		if (err != KErrNone)
			continue; 
		
		char d[3];
		d[0] = 'A'+driveNumber;
		d[1] = ':';
		d[2] = '\0';
		
#ifndef EKA2
		TInt freeSpace = (volumeInfo.iFree).GetTInt();
#else
		TInt freeSpace = (volumeInfo.iFree);
#endif /*EKA2*/

		add_assoc_long(return_value, d, freeSpace);
	}
	fsSession.Close();
}



/**
 * <array> s60_sysinfo_display_twips()
 * Returns the twips of the device's display.
 */
PHP_FUNCTION(s60_sysinfo_display_twips)
{
	GET_FUNCTION_ARGS_0();
	TInt error = KErrNone;
	TInt xValue;
	TInt yValue;
	
	error = HAL::Get(HALData::EDisplayXTwips, xValue);
	RETURN_IF_SYMBIAN_ERROR(error);
	
	error = HAL::Get(HALData::EDisplayYTwips, yValue);
	RETURN_IF_SYMBIAN_ERROR(error);
	
	array_init(return_value);
	add_next_index_long(return_value, xValue);
	add_next_index_long(return_value, yValue);
}

/**
 * <array> s60_sysinfo_display_pixels()
 * Returns the pixels in the device's display.
 */
PHP_FUNCTION(s60_sysinfo_display_pixels)
{
	GET_FUNCTION_ARGS_0();
	TInt error = KErrNone;
	TInt xValue;
	TInt yValue;
	
	error = HAL::Get(HALData::EDisplayXPixels, xValue);
	RETURN_IF_SYMBIAN_ERROR(error);
	
	error = HAL::Get(HALData::EDisplayYPixels, yValue);
	RETURN_IF_SYMBIAN_ERROR(error);
	
	array_init(return_value);
	add_next_index_long(return_value, xValue);
	add_next_index_long(return_value, yValue);
}

/**
 * <int> s60_sysinfo_battery()
 * Returns the battery level left in the device.
 */
PHP_FUNCTION(s60_sysinfo_battery)
{
	GET_FUNCTION_ARGS_0();
#ifdef S60_EMULATOR
	RETURN_LONG(0);
#else
#ifndef EKA2
	RSystemAgent systemAgent;
	TInt error = systemAgent.Connect();
	RETURN_IF_SYMBIAN_ERROR(error);
	
	// Get battery value:
	TInt batteryValue = systemAgent.GetState(KUidBatteryBars); 
	systemAgent.Close();
	RETURN_LONG(batteryValue);
#else
	TInt error = KErrNone;
	CTelephony* telephony = NULL;
	TRAP(error,telephony = CTelephony::NewL());
	TRequestStatus status;
	
	CTelephony::TBatteryInfoV1 batteryInfo;
	CTelephony::TBatteryInfoV1Pckg batteryInfoPkg(batteryInfo);
	
//	Py_BEGIN_ALLOW_THREADS
	CAsyncCallHandlerSysInfo* asyncCallHandler = NULL;

	TRAP(error,{
		asyncCallHandler = CAsyncCallHandlerSysInfo::NewL(*telephony);
		asyncCallHandler->GetBatteryInfo(batteryInfoPkg,status);
	});
	
	delete asyncCallHandler;
//	Py_END_ALLOW_THREADS
	delete telephony;
	
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_LONG(batteryInfo.iChargeLevel);
	
#endif /*EKA2*/
#endif /*S60_EMULATOR*/
}

/**
 * <int> s60_sysinfo_battery_max()
 * Returns the maximum battery level in the device.
 */
PHP_FUNCTION(s60_sysinfo_battery_max)
{
	GET_FUNCTION_ARGS_0();
#ifdef S60_EMULATOR
	RETURN_LONG(1);
#elif !defined(EKA2)
	RETURN_LONG(7);
#else
	RETURN_LONG(100);
#endif
}

/**
 * <int> s60_sysinfo_signal()
 * Returns the signal strength currently.
 */
PHP_FUNCTION(s60_sysinfo_signal)
{
	GET_FUNCTION_ARGS_0();
#ifdef S60_EMULATOR
	RETURN_LONG(0);
#else
#ifndef EKA2
	RSystemAgent systemAgent;
	TInt error = systemAgent.Connect();
	RETURN_IF_SYMBIAN_ERROR(error);
	
	// Get network value:
	TInt networkValue = systemAgent.GetState(KUidNetworkBars); 
	systemAgent.Close();
	RETURN_LONG(networkValue);
#else 
	TInt error = KErrNone;
	CTelephony* telephony = NULL;
	TRAP(error,telephony = CTelephony::NewL());
	TRequestStatus status;
	
	CTelephony::TSignalStrengthV1 signalStrength;	
	CTelephony::TSignalStrengthV1Pckg signalStrengthPkg(signalStrength);
	
//	Py_BEGIN_ALLOW_THREADS
	CAsyncCallHandlerSysInfo* asyncCallHandler = NULL;
	
	TRAP(error,{
		asyncCallHandler = CAsyncCallHandlerSysInfo::NewL(*telephony);
		asyncCallHandler->GetSignalStrength(signalStrengthPkg,status);
	});
	
	delete asyncCallHandler;
	
//	Py_END_ALLOW_THREADS
	
	delete telephony;
	
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_LONG(signalStrength.iSignalStrength);
#endif /*EKA2*/
#endif /*S60_EMULATOR*/
}

/**
 * <int> s60_sysinfo_signal_max()
 * Returns the maximum signal strength in the device.
 */
PHP_FUNCTION(s60_sysinfo_signal_max)
{
	GET_FUNCTION_ARGS_0();
#ifdef S60_EMULATOR
	RETURN_LONG(1);
#elif !defined(EKA2)
	RETURN_LONG(7);
#else
	RETURN_LONG(100); /* XXX I'm not sure about this one! */
#endif
}

/**
 * <string> s60_sysinfo_imei()
 * Returns the imei code.
 */
PHP_FUNCTION(s60_sysinfo_imei)
{
	GET_FUNCTION_ARGS_0();
#ifdef S60_EMULATOR
	// Return a fake IMEI when on emulator
	RETURN_STRING("000000000000000", 1);
#else
#ifndef EKA2
	// This only works on target machine
	TPlpVariantMachineId imei;
	TRAPD(error,(PlpVariant::GetMachineIdL(imei)));
	RETURN_IF_SYMBIAN_ERROR(error);
	
	error = ZVAL_SYMBIAN_STRING(return_value, imei);
	RETURN_IF_SYMBIAN_ERROR(error);
#else 
	TInt error = KErrNone;
	CTelephony* telephony = NULL;
	TRAP(error, telephony = CTelephony::NewL());
	RETURN_IF_SYMBIAN_ERROR(error);
	
	TRequestStatus status;
	
	CTelephony::TPhoneIdV1 phoneId;	
	CTelephony::TPhoneIdV1Pckg phoneIdPkg(phoneId);
	
//	Py_BEGIN_ALLOW_THREADS
	CAsyncCallHandlerSysInfo* asyncCallHandler = NULL;
	
	TRAP(error,{
		asyncCallHandler = CAsyncCallHandlerSysInfo::NewL(*telephony);
		asyncCallHandler->GetPhoneId(phoneIdPkg,status);
	});
	
	delete asyncCallHandler;
	
//	Py_END_ALLOW_THREADS
	
	delete telephony;
	
	RETURN_IF_SYMBIAN_ERROR(error);
	
	error = ZVAL_SYMBIAN_STRING(return_value, phoneId.iSerialNumber);
	RETURN_IF_SYMBIAN_ERROR(error);
#endif /*EKA2*/
#endif /*S60_EMULATOR*/
}

/**
 * <bool> s60_sysinfo_in_emulator()
 * Returns true if called from emulator.
 */
PHP_FUNCTION(s60_sysinfo_in_emulator)
{
	GET_FUNCTION_ARGS_0();
#ifdef S60_EMULATOR
	RETURN_TRUE;
#else
	RETURN_FALSE;
#endif	
}

PHP_FUNCTION(s60_sysinfo_current_profile)
{
	CRepository* cr = CRepository::NewLC( KCRUidProfileEngine );
 	TInt value = 0;
 	TInt error = cr->Get( KProEngActiveProfile, value );
 	CleanupStack::PopAndDestroy();
 	
 	RETURN_IF_SYMBIAN_ERROR(error);
 	RETURN_LONG( value );

}

PHP_FUNCTION(s60_message_query)
{
	char* msg = NULL;
	int len = 0;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &msg, &len ) == FAILURE )
	{
		RETURN_NULL();
	}
	
	TBuf8<255> msgBuf;
	msgBuf.Copy(  (TUint8*)msg, len );
	
	TBuf<255> msgBuf16;
	msgBuf16.Copy( msgBuf );
	
	CAknGlobalMsgQuery* dlg = CAknGlobalMsgQuery::NewL();
	TRequestStatus status;
	
	TRAPD( err, dlg->ShowMsgQueryL( status, msgBuf16, R_AVKON_SOFTKEYS_OK_CANCEL , _L(" "), _L(""), 0,-1, CAknQueryDialog::EConfirmationTone ) );
	
	User::WaitForRequest( status );
	
	delete dlg;
	
	RETURN_LONG( status.Int() );
}

} /* extern "C" */
