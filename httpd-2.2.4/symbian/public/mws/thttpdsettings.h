#ifndef mws_thttpdsettings_h
#define mws_thttpdsettings_h
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

#include "mws/httpd.h"
#include <s32file.h>


namespace Mws
{
    namespace HttpdSettings
    {
        enum TVersion
            {
            EVersion1 = 1
            };
    }

    class THttpdSettings1
        {
    public:
        enum 
            {
            KVersion = HttpdSettings::EVersion1,
            };

        THttpdSettings1()
            : iMinHeapSize(0),
              iMaxHeapSize(0),
              iStackSize(0)
            {
            }

        THttpdSettings1(TInt32 aMinHeapSize, 
                        TInt32 aMaxHeapSize, 
                        TInt32 aStackSize)
            : iMinHeapSize(aMinHeapSize),
              iMaxHeapSize(aMaxHeapSize),
              iStackSize(aStackSize)
            {
            }

        ~THttpdSettings1()
            {
            }

        TInt32 MinHeapSize() const 
            { 
            return iMinHeapSize; 
            }
        
        void SetMinHeapSize(TInt32 aMinHeapSize) 
            { 
            iMinHeapSize = aMinHeapSize; 
            }
        
        TInt32 MaxHeapSize() const 
            { 
            return iMaxHeapSize; 
            }
        
        void SetMaxHeapSize(TInt32 aMaxHeapSize) 
            { 
            iMaxHeapSize = aMaxHeapSize; 
            }

        TInt32 StackSize() const
            {
            return iStackSize;
            }

        void SetStackSize(TInt32 aStackSize)
            {
            iStackSize = aStackSize;
            }

        struct TBoolTag
            {
            };

        TBool IsValid() const { return true; }

        operator TBoolTag* () const
            {
            return reinterpret_cast<TBoolTag*>(IsValid());
            }

        void WriteDataL(RWriteStream& ahStream) const
            {
            ahStream.WriteInt32L(iMinHeapSize);
            ahStream.WriteInt32L(iMaxHeapSize);        
            ahStream.WriteInt32L(iStackSize);
            }
        
        void ExternalizeL(RWriteStream& ahStream) const
            {
            ahStream.WriteInt32L(KVersion);
            
            WriteDataL(ahStream);
            }

        void ReadDataL (RReadStream& ahStream)
            {
            iMinHeapSize = ahStream.ReadInt32L();
            iMaxHeapSize = ahStream.ReadInt32L();
            iStackSize   = ahStream.ReadInt32L();
            }

        void InternalizeL(RReadStream& ahStream)
            {
            TInt32
                version = ahStream.ReadInt32L();
            
            if (version != KVersion)
                User::Leave(KErrCorrupt);
            
            ReadDataL(ahStream);
            }
        
    private:
        TInt32 iMinHeapSize;
        TInt32 iMaxHeapSize;
        TInt32 iStackSize;        
        };


    typedef THttpdSettings1 THttpdSettings;
}

#endif
