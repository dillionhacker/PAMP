#ifndef mws_carguments_h
#define mws_carguments_h
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

#include "mws/httpdutil.h"
#include <s32file.h>


namespace Mws
{

    namespace Arguments
    {
        enum TVersion
            {
            EVersion1 = 1
            };
    }

    
    class CArguments1 : public ::CBase
        {
    public:
        enum
            {
            KVersion = Mws::Arguments::EVersion1
            };

        IMPORT_C static CArguments1* NewL();
        IMPORT_C static CArguments1* NewLC();
        
        IMPORT_C ~CArguments1();

        IMPORT_C const TDesC& Get() const;

        IMPORT_C void SetL(const TDesC& aValue);
        
        IMPORT_C void ReadDataL (RReadStream& ahStream);
        IMPORT_C void WriteDataL(RWriteStream& ahStream) const;
        
        IMPORT_C void ExternalizeL(const TDesC& aPath) const;
        IMPORT_C void ExternalizeL(RWriteStream& ahStream) const;
        
        IMPORT_C void InternalizeL(const TDesC& aPath);
        IMPORT_C void InternalizeL(RReadStream& ahStream);
        
        IMPORT_C void CopyL(const CArguments1& aRhs);
        
    protected:
        IMPORT_C CArguments1();
        
        IMPORT_C void ConstructL();
        
    private:
        CArguments1(const CArguments1&);
        CArguments1& operator = (const CArguments1&);
        
    private:
        HBufC* ipValue;
        };


    typedef CArguments1 CArguments;

} // namespace Mws
    
#endif
