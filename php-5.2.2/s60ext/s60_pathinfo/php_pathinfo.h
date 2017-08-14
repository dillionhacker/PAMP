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

#ifndef __MOD_PATHINFO_H__
#define __MOD_PATHINFO_H__

PHP_MINIT_FUNCTION(s60_pathinfo);
PHP_MSHUTDOWN_FUNCTION(s60_pathinfo);
PHP_RINIT_FUNCTION(s60_pathinfo);
PHP_RSHUTDOWN_FUNCTION(s60_pathinfo);
PHP_MINFO_FUNCTION(s60_pathinfo);

PHP_FUNCTION( s60_rom_root_path );
PHP_FUNCTION( s60_phone_memory_root_path );
PHP_FUNCTION( s60_memory_card_path );
PHP_FUNCTION( s60_games_path );
PHP_FUNCTION( s60_installs_path );
PHP_FUNCTION( s60_others_path );
PHP_FUNCTION( s60_videos_path );
PHP_FUNCTION( s60_images_path );
PHP_FUNCTION( s60_pictures_path );
PHP_FUNCTION( s60_gms_pictures_path );
PHP_FUNCTION( s60_mms_background_images_path );
PHP_FUNCTION( s60_presence_logos_path );
PHP_FUNCTION( s60_sounds_path );
PHP_FUNCTION( s60_digital_sounds_path );
PHP_FUNCTION( s60_simple_sounds_path );
PHP_FUNCTION( s60_images_thumbnail_path );
PHP_FUNCTION( s60_memory_card_contacts_path );

extern zend_module_entry s60_pathinfo_module_entry;
#define phpext_s60_pathinfo_ptr &s60_pathinfo_module_entry

#endif
