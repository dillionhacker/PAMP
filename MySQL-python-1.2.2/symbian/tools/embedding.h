#ifndef httpd_symbian_tools_embedding_h
#define httpd_symbian_tools_embedding_h
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


/* Stringizes sis */
#define EMBEDDED_SIS3(sis) #sis

/* Concatenates module and tail and stringizes the result. */
#define EMBEDDED_SIS2(module, tail) EMBEDDED_SIS3(module ## tail)

/* Concatenates module and tail and stringizes the result. 'module' and
   'tail' can themselves be macros.
*/
#define EMBEDDED_SIS(module, tail)  EMBEDDED_SIS2(module, tail)

#endif
