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
#ifndef __MOD_LOCATION_H__
#define __MOD_LOCATION_H__

PHP_MINIT_FUNCTION(s60_location);
PHP_MSHUTDOWN_FUNCTION(s60_location);
PHP_RINIT_FUNCTION(s60_location);
PHP_RSHUTDOWN_FUNCTION(s60_location);
PHP_MINFO_FUNCTION(s60_location);

PHP_FUNCTION( s60_loc_available );
PHP_FUNCTION( s60_loc_position );
PHP_FUNCTION( s60_loc_speed );

PHP_FUNCTION( s60_loc_extract_position_data );

extern zend_module_entry s60_location_module_entry;
#define phpext_s60_location_ptr &s60_location_module_entry




#endif
