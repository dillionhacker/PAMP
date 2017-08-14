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

#ifndef APACHE_OS_H
#define APACHE_OS_H

#include "apr.h"
#include "ap_config.h"

#ifndef PLATFORM
/*
 * Plain "S60" would be better, now that some S60 specific OpenC 
 * libraries are used, but Python uses "symbian_s60" and they
 * had better be the same.
 */ 
#define PLATFORM "symbian_s60"
#endif

#define CASE_BLIND_FILESYSTEM
#define HAVE_DRIVE_LETTERS

/*
 * Symbian has SO_ACCEPTFILTER, but "httpready" is not recognized.
 * By setting ACCEPT_FILTER_NAME to "none" we can prevent it from
 * being unsuccessfully set and complained about.
 * 
 */ 

#define ACCEPT_FILTER_NAME "none"

#endif	/* !APACHE_OS_H */
