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

extern "C"
{
#include "apr_strings.h"
#include "ap_config.h"
}


extern "C"
{

AP_DECLARE(int) ap_lookup_table_symbol(const ap_export_entry* entries,
                                       int                    count,
                                       const char*            name,
                                       void**                 symbol)
{
    const ap_export_entry
        *i   = entries,
        *end = i + count;
    
    while (i != end) {
        if (apr_strnatcmp(name, i->name) == 0)
            {
                *symbol = i->symbol;

                return APR_SUCCESS;
            }
        
        ++i;
    }

    return APR_ENOENT;
}

}
