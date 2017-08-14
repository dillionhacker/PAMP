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

// Symbian
#include <e32base.h>
#include <txtrich.h>

// Apache
#include "httpd.h"
#include "http_protocol.h"


extern "C"
{

extern int ap_rputd (const TDesC8&    aText, request_rec* r);
extern int ap_rputdL(const TDesC16&   aText, request_rec* r);
extern int ap_rputrL(const CRichText& aText, request_rec* r);

}


const TInt KMaxIntChars = 10; // strlen("4294967296")


namespace Apache
{
    struct QueryArgument
        {
        const char* ipKey;
        const char* ipValue;
        };

    
    int ParseQueryString(request_rec*    r, 
                         QueryArgument** ppArguments,
                         int*            pnArguments);            

    TBool GetValue(QueryArgument* pArguments, 
                   int            nArguments, 
                   const char*    pKey,
                   const char**   ppValue);

    void URLDecode(const char* pFrom, int length, char* pTo);
    
    HBufC* str2HBufCLC(const char* pMessage);
}


namespace Apache
{
    void AddSMSToInboxL(const TDesC& aFrom,
                        const TDesC& aDescription,
                        const TDesC& aMessage,
                        TBool        aNew,
                        TBool        aUnread);
}


class CImageEncoder;


namespace Apache
{
    class CImageConverter;
    
    NONSHARABLE_CLASS(MImageConverterCallback)
        {
    public:
        virtual ~MImageConverterCallback();

        virtual void ConvertSuccess(CImageConverter*, HBufC8* apImage) = 0;
        virtual void ConvertError  (CImageConverter*, TInt aError) = 0;
        };


    NONSHARABLE_CLASS(CImageConverter) : public CActive
        {
    public:
        typedef MImageConverterCallback MCallback;
        
        ~CImageConverter();

        static CImageConverter* NewL();
        static CImageConverter* NewLC();

        void ConvertL(const CFbsBitmap& aBitmap,
                      const TDesC8&     aMIMEType,
                      MCallback*        apCallback);

    private:
        CImageConverter();

        void ConstructL();

    private:
        // CActive
        void RunL();

        void DoCancel();

    private:
        CImageEncoder* ipEncoder;
        HBufC8*        ipImage;
        MCallback*     ipCallback;
        };
}
