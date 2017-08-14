#ifndef mod_python_symbian_config_symbian_mod_python_builtin_h
#define mod_pythin_symbian_config_symbian_mod_python_builtin_h
/* Copyright 2008 Nokia Corporation
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

//
// NOTE: This file is included by mmp-processor as well.
//

//
// If you want to have a module built-in, then turn the undef into
// a define.
//
// NOTE: You need to modify 
//       - httpd-2.2/os/symbian/modules.c
//       - httpd-2.2/build/libhttpd.mmp
//       as well, if you want this module to be built in.
//
// NOTE: This is primarily intended for making on-target debugging
//       easier. Normally there is NO reason to build in a module.
// 

#undef HAVE_MOD_PYTHON_BUILTIN

#endif
