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
 * Position API for PHP. In order to use this, Location Acquisition Daemon 
 * (LAD) needs to be running on the system.
 *   
 */
#include <e32base.h>
#include <e32property.h>
#include <lbs.h>

extern "C"
{
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_location.h"

const TUid KPositionCategory = { 0x2000E483 };

enum TPositionKeys
{
	EKeyLatitude = 1,
	EKeyLongitude,
	EKeyAltitude,
	EKeyAccuracyHoriz,
	EKeyAccuracyVert,
	EKeyPositionHistory,
	EKeyPosition
};

zend_function_entry s60_location_functions[] = {
  PHP_FE( s60_loc_available, NULL )
  PHP_FE( s60_loc_position, NULL )
  PHP_FE( s60_loc_speed, NULL )
  PHP_FE( s60_loc_extract_position_data, NULL )
  
  {NULL, NULL, NULL}
};

zend_module_entry s60_location_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"s60_location",
	s60_location_functions,
	PHP_MINIT(s60_location),
	PHP_MSHUTDOWN(s60_location),
	PHP_RINIT(s60_location),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(s60_location),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(s60_location),
#if ZEND_MODULE_API_NO >= 20010901
	"1.0", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};

int create_position_object( zval* position_arr, TPosition& position );

PHP_MINIT_FUNCTION(s60_location)
{
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(s60_location)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}

PHP_RINIT_FUNCTION(s60_location)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(s60_location)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(s60_location)
{
	
}

PHP_FUNCTION( s60_loc_available )
{
	
}

#define RETURN_POSITION( p ) \
{ \
	array_init(return_value); \
	add_assoc_double(return_value, "latitude", position().Latitude() ); \
	add_assoc_double(return_value, "longitude", position().Longitude() ); \
	add_assoc_double(return_value, "altitude", position().Altitude() ); \
	add_assoc_double(return_value, "time", position().Time().Int64() ); \
	add_assoc_double(return_value, "horizontal_accuracy", position().HorizontalAccuracy() ); \
	add_assoc_double(return_value, "vertical_accuracy", position().VerticalAccuracy() ); \
}

PHP_FUNCTION( s60_loc_position )
{
	TPckgBuf<TPosition> position;
	
	TInt err = 0;
	
	if( ( err = RProperty::Get( KPositionCategory, EKeyPosition, position ) ) == KErrNone )
	{
		RETURN_POSITION( position() );
	}
	else
	{
		RETURN_LONG( err );
	}
	
}

PHP_FUNCTION( s60_loc_extract_position_data )
{
	char* positionData = NULL;
	int len = 0;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &positionData, &len ) == FAILURE )
	{
		RETURN_NULL();
	}
	
	TPckgBuf<TPosition> position;
	position.Copy( (TUint8*)positionData, len );
	
	RETURN_POSITION( position() );
}

PHP_FUNCTION( s60_loc_speed )
{
	zval* position1 = NULL;
	zval* position2 = NULL;
	
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "aa", &position1, &position2 ) == FAILURE )
	{
		RETURN_NULL();
	}
	
	TPosition sp, ep;
	
	if( create_position_object( position1, sp ) == FAILURE )
		RETURN_NULL();
	
	if( create_position_object( position2, ep ) == FAILURE )
		RETURN_NULL();
	
	TReal32 speed;
	
	if( sp.Speed( ep, speed ) == KErrArgument )
		RETURN_LONG( KErrArgument );
	
	RETURN_DOUBLE( speed );
}


#define GET_POS_VAL( var_name, data ) if( zend_hash_find( arr_hash, var_name, sizeof( var_name ), (void**)&data ) == FAILURE ) return FAILURE;

int create_position_object( zval* position_arr, TPosition& position )
{
	HashTable *arr_hash;
    HashPosition pointer;

	zval** data1, **data2, **data3;
    
    arr_hash = Z_ARRVAL_P( position_arr );
    
   	GET_POS_VAL( "latitude", data1 );
   	GET_POS_VAL( "longitude", data2 );
   	GET_POS_VAL( "altitude", data3 );
   	
   	if( Z_TYPE_PP( data1 ) == IS_DOUBLE && 
   		Z_TYPE_PP( data2 ) == IS_DOUBLE && 
   		Z_TYPE_PP( data3 ) == IS_DOUBLE )
   	{
   		double latitude = Z_DVAL_PP( data1 );
   		double longitude = Z_DVAL_PP( data2 );
   		double altitude = Z_DVAL_PP( data3 );
   		
   		php_printf( "la:%f lo:%f al:%f<br>", latitude, longitude, altitude  );
   		
   		position.SetCoordinate( latitude, longitude );
   	}
   	
   	GET_POS_VAL( "time", data1 );
   	
   	if( Z_TYPE_PP( data1 ) == IS_DOUBLE )
   	{
   		TReal64 time = Z_DVAL_PP( data1 );
   		
   		php_printf( "t:%f<br>", time );
   		
   		position.SetTime( TTime( time ) );
   	}
   
   	GET_POS_VAL( "horizontal_accuracy", data1 );
   	GET_POS_VAL( "vertical_accuracy", data2 );
   
   	if( Z_TYPE_PP( data1 ) == IS_DOUBLE && 
   		Z_TYPE_PP( data2 ) == IS_DOUBLE )
   	{
   		TReal32 ha = Z_DVAL_PP( data1 );
   		TReal32 va = Z_DVAL_PP( data2 );
   		
   		php_printf( "ha:%f va:%f<br>", ha, va  );
   	
   		position.SetAccuracy( ha, va );
   	}
   	
   	return SUCCESS;
}

}
