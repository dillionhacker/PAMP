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

#include <random.h>
#include "apr_general.h"


namespace
{
    apr_status_t GenerateRandomBytesL(TPtr8& aBuffer)
        {
        RRandomSession
            hSession;
        
        hSession.ConnectL();

        CleanupClosePushL(hSession);

        hSession.GetRandom(aBuffer);

        CleanupStack::PopAndDestroy(); // hSession.Close();

        return APR_SUCCESS;
        }
    

    apr_status_t GenerateRandomBytes(TPtr8& aBuffer)
        {
        apr_status_t
            rc = APR_SUCCESS;
        
        TRAPD(error, rc = GenerateRandomBytesL(aBuffer));

        switch (error)
            {
            case KErrNone:
                break;

            case KErrNoMemory:
                rc = APR_ENOMEM;
                break;
                
            default:
                rc = APR_EGENERAL;
            }

        return rc;
        }
}


extern "C"
{

APR_DECLARE(apr_status_t) apr_generate_random_bytes(unsigned char* buf,
                                                    apr_size_t     length)
{
    TPtr8
        buffer(buf, length);

    return GenerateRandomBytes(buffer);
}

} // extern "C"
