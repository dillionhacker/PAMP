#ifndef httpds_request_h
#define httpds_request_h
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

#include <e32base.h>


namespace HttpdS
{
    enum
        {
        KIndexReturnValue = 0
        };


    template<class T>
    inline void ReturnDataL(const RMessage2& aMessage, const T& aData)
        {
        TPckgC<T>
            result(aData);
        
        aMessage.WriteL(KIndexReturnValue, result);
        
        aMessage.Complete(KErrNone);
        }


    template<class T>
    inline void ReadDataL(const RMessage2& aMessage, TInt aIndex, T& aData)
        {
        TPckg<T>
            result(aData);

        aMessage.ReadL(aIndex, result);
        }



    struct AsyncRequestBase
        {
        AsyncRequestBase()
            : iPending(EFalse)
            {}

        TBool Pending() const { return iPending; }

        void SetPending(const RMessage2& aMessage)
            {
            ASSERT(!Pending());
            
            iPending = ETrue;
            iMessage = aMessage;
            }

        void Cancel(const RMessage2& aByRequest)
            {
            ASSERT(Pending());

            iPending = EFalse;
            iMessage.Complete(KErrCancel);
            aByRequest.Complete(KErrNone);
            }
        
        RMessage2 iMessage;
        TBool     iPending;
        };

        
    template<typename T>
    struct AsyncRequest : public AsyncRequestBase
        {
        typedef T value_type;
        
        T iData;

        const T& Data() const
            {
            return iData;
            }
        
        
        void Update(const T& aData)
            {
            iData = aData;
            }


        static void ReturnL(const RMessage2& aMessage, const T& aData)
            {
            ReturnDataL(aMessage, aData);
            }


        void ReturnL(const T& aData)
            {
            ASSERT(Pending());
            
            iPending = EFalse;
            iData    = aData;

            ReturnL(iMessage, iData);
            }
        };
}

#endif
