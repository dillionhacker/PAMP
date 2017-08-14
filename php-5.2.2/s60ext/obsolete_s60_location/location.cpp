/**
 * ====================================================================
 *  PHP API to location information, modified from corresponding
 *  Python API.
 * --------------------------------------------------------------------
 *
 * Copyright (c) 2006 Nokia Corporation
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
 * ====================================================================
 */

/**
//EXAMPLE


<pre><?php

var_dump(s60_location());

?></pre>

*/


#include <eikenv.h>
#include <e32std.h>

#include <utf.h> // charconv.lib

#ifndef EKA2
//#include <etelbgsm.h>
#else
#include <etel.h>
#include <etel3rdparty.h>
//#include <e32cmn.h>
#include "asynccallhandler.h"
#endif

#include "s60ext_tools.h"

extern "C" {
	
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h" 
 	
#include "location.h"

BEGIN_MODULE_FUNCTIONS(s60_location)
    PHP_FE(s60_location, NULL)
END_MODULE_FUNCTIONS()

DEFINE_MODULE(s60_location, "1.0", NULL, NULL, NULL, NULL)


/**
 * <array> s60_location()
 * Returns information on phone location in an associative array.
 * Array keys:
 * "location area code"
 * "cell id"
 * "area known"
 *
 * Feature only supported on S60 version 3.x!
 */
PHP_FUNCTION(s60_location)
{
	GET_FUNCTION_ARGS_0();
#ifndef EKA2
/*
	_LIT (KTsyName, "phonetsy.tsy");
	TInt error = KErrNone;
	
	TInt enumphone = 1;
	RTelServer	 server;
	RBasicGsmPhone phone;
	RTelServer::TPhoneInfo info;
	MBasicGsmPhoneNetwork::TCurrentNetworkInfo NetworkInfo;
	
	error = server.Connect();
	RETURN_IF_SYMBIAN_ERROR(error);
	
	error = server.LoadPhoneModule(KTsyName);
	RETURN_IF_SYMBIAN_ERROR(error);
	
	error = server.EnumeratePhones(enumphone);
	RETURN_IF_SYMBIAN_ERROR(error);
	
	if (enumphone < 1)
		RETURN_ERROR("Not found");
	
	error = server.GetPhoneInfo(0, info);
	RETURN_IF_SYMBIAN_ERROR(error);
	
	error = phone.Open(server, info.iName);
	RETURN_IF_SYMBIAN_ERROR(error);
	
	error = phone.GetCurrentNetworkInfo(NetworkInfo);
	RETURN_IF_SYMBIAN_ERROR(error);
		       
	phone.Close();
	server.Close();
	
	array_init(return_value);
	add_assoc_long(return_value, "location area code", NetworkInfo.iLocationAreaCode);
	add_assoc_long(return_value, "cell id", NetworkInfo.iCellId);
	add_assoc_long(return_value, "mcc", NetworkInfo.iNetworkInfo.iId.iMCC);
	add_assoc_long(return_value, "mnc", NetworkInfo.iNetworkInfo.iId.iMNC);
	*/
	RETURN_ERROR("Feature not supported");
#else
	TInt error = KErrNone;
	CTelephony* telephony = NULL;
	TRAP(error, {
		telephony = CTelephony::NewL();
	});
    RETURN_IF_SYMBIAN_ERROR(error);
	
	TRequestStatus status;
	CTelephony::TNetworkInfoV1 networkInfo;
	
	CTelephony::TNetworkInfoV1Pckg networkInfoPkg(networkInfo);
	
	//Py_BEGIN_ALLOW_THREADS
	CAsyncCallHandler* asyncCallHandler = NULL;
	
	TRAP(error,{
		asyncCallHandler = CAsyncCallHandler::NewL(*telephony);
		asyncCallHandler->MakeAsyncCall(networkInfoPkg,status);
	});
	
	delete asyncCallHandler;
	//Py_END_ALLOW_THREADS
	
	delete telephony;
	
    RETURN_IF_SYMBIAN_ERROR(error);
	
	array_init(return_value);
	/*
	TBuf8<256> utf8str;  
	error = CnvUtfConverter::ConvertFromUnicodeToUtf8(utf8str, networkInfo.iCountryCode);
	RETURN_IF_SYMBIAN_ERROR(error);
	add_assoc_stringl(return_value, "country code", (char*)utf8str.Ptr(), utf8str.Size(), 1);
	
	error = CnvUtfConverter::ConvertFromUnicodeToUtf8(utf8str, networkInfo.iNetworkId);
    RETURN_IF_SYMBIAN_ERROR(error);
	add_assoc_stringl(return_value, "network id", (char*)utf8str.Ptr(), utf8str.Size(), 1);
	*/
	add_assoc_long(return_value, "location area code", networkInfo.iLocationAreaCode);
	add_assoc_long(return_value, "cell id", networkInfo.iCellId);
	add_assoc_long(return_value, "area known", networkInfo.iAreaKnown);
#endif
}

} /* extern "C" */
