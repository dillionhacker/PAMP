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

#ifndef __EXT_LOG_H__
#define __EXT_LOG_H__

extern zend_module_entry s60_log_module_entry;
#define phpext_s60_log_ptr &s60_log_module_entry


PHP_MINIT_FUNCTION(s60_log);
PHP_MSHUTDOWN_FUNCTION(s60_log);

PHP_FUNCTION( set_filter );
PHP_FUNCTION( first_entry );
PHP_FUNCTION( next_entry );
PHP_FUNCTION( last_entry );
PHP_FUNCTION( prev_entry );


#endif
