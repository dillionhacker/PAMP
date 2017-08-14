/* Copyright 2006 Nokia Corporation
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

#include "httpd.h"
#include "http_config.h"


extern "C"
{
    extern module symbian_calendar_module;
    extern module symbian_contacts_module;
    extern module symbian_dos_module;
    extern module symbian_inbox_module;
    extern module symbian_logs_module;
    extern module symbian_messages_module;
    extern module symbian_proximity_module;


AP_EXPORTS_BEGIN()
AP_EXPORT_SYMBOL(symbian_calendar_module)
AP_EXPORT_SYMBOL(symbian_contacts_module)
AP_EXPORT_SYMBOL(symbian_dos_module)
AP_EXPORT_SYMBOL(symbian_inbox_module)
AP_EXPORT_SYMBOL(symbian_logs_module)
AP_EXPORT_SYMBOL(symbian_messages_module)
AP_EXPORT_SYMBOL(symbian_proximity_module)
AP_EXPORTS_END()

}
