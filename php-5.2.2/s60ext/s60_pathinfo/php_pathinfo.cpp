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

#include <e32base.h>
#include <pathinfo.h> 

extern "C"
{
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_pathinfo.h"

zend_function_entry s60_pathinfo_functions[] = {
  PHP_FE( s60_rom_root_path, NULL )
  PHP_FE( s60_phone_memory_root_path, NULL )
  PHP_FE( s60_memory_card_path, NULL )
  PHP_FE( s60_games_path, NULL )
  PHP_FE( s60_installs_path, NULL )
  PHP_FE( s60_others_path, NULL )
  PHP_FE( s60_videos_path, NULL )
  PHP_FE( s60_images_path, NULL )
  PHP_FE( s60_pictures_path, NULL )
  PHP_FE( s60_gms_pictures_path, NULL )
  PHP_FE( s60_mms_background_images_path, NULL )
  PHP_FE( s60_presence_logos_path, NULL )
  PHP_FE( s60_sounds_path, NULL )
  PHP_FE( s60_digital_sounds_path, NULL )
  PHP_FE( s60_simple_sounds_path, NULL )
  PHP_FE( s60_images_thumbnail_path, NULL )
  PHP_FE( s60_memory_card_contacts_path, NULL )
  {NULL, NULL, NULL}
};

zend_module_entry s60_pathinfo_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"extname",
	s60_pathinfo_functions,
	PHP_MINIT(s60_pathinfo),
	PHP_MSHUTDOWN(s60_pathinfo),
	PHP_RINIT(s60_pathinfo),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(s60_pathinfo),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(s60_pathinfo),
#if ZEND_MODULE_API_NO >= 20010901
	"1.0", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};

#define RETURN_PATH( func ) TBuf8<255> path; \
							path.Copy( func ); \
							ZVAL_STRINGL( return_value, (char*)path.PtrZ(), path.Length(), 1 );

PHP_MINIT_FUNCTION(s60_pathinfo)
{
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(s60_pathinfo)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}

PHP_RINIT_FUNCTION(s60_pathinfo)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(s60_pathinfo)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(s60_pathinfo)
{
	
}

PHP_FUNCTION( s60_rom_root_path )
{
	RETURN_PATH( PathInfo::RomRootPath() );
}

PHP_FUNCTION( s60_phone_memory_root_path )
{
	RETURN_PATH( PathInfo::PhoneMemoryRootPath() );
}

PHP_FUNCTION( s60_memory_card_path )
{
	RETURN_PATH( PathInfo::MemoryCardRootPath() );
}

PHP_FUNCTION( s60_games_path )
{
	RETURN_PATH( PathInfo::GamesPath() );
}

PHP_FUNCTION( s60_installs_path )
{
	RETURN_PATH( PathInfo::InstallsPath() );
}

PHP_FUNCTION( s60_others_path )
{
	RETURN_PATH( PathInfo::OthersPath() );
}

PHP_FUNCTION( s60_videos_path )
{
	RETURN_PATH( PathInfo::VideosPath() );
}

PHP_FUNCTION( s60_images_path )
{
	RETURN_PATH( PathInfo::ImagesPath() );
}

PHP_FUNCTION( s60_pictures_path )
{
	RETURN_PATH( PathInfo::PicturesPath() );
}

PHP_FUNCTION( s60_gms_pictures_path )
{
	RETURN_PATH( PathInfo::GmsPicturesPath() );
}

PHP_FUNCTION( s60_mms_background_images_path )
{
	RETURN_PATH( PathInfo::MmsBackgroundImagesPath() );
}

PHP_FUNCTION( s60_presence_logos_path )
{
	RETURN_PATH( PathInfo::PresenceLogosPath() );
}

PHP_FUNCTION( s60_sounds_path )
{
	RETURN_PATH( PathInfo::SoundsPath() );
}

PHP_FUNCTION( s60_digital_sounds_path )
{
	RETURN_PATH( PathInfo::DigitalSoundsPath() );
}

PHP_FUNCTION( s60_simple_sounds_path )
{
	RETURN_PATH( PathInfo::SimpleSoundsPath() );
}

PHP_FUNCTION( s60_images_thumbnail_path )
{
	RETURN_PATH( PathInfo::ImagesThumbnailPath() );	
}

PHP_FUNCTION( s60_memory_card_contacts_path )
{
	RETURN_PATH( PathInfo::MemoryCardContactsPath() );
}
}
