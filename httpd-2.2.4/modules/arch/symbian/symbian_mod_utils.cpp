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

#include "symbian_mod_utils.h"
#include <mtclreg.h>
#include <msvapi.h>
#include <msvids.h>
#include <smsclnt.h>
#include <smut.h>
// Should the inclusion of the following two files fail, you are most
// likely using S60v2.0 from which they have inadvertenty(?) been
// left out.
//
// From the Symbian web-site look for FAQ-0980 and follow the 
// instructions there. Last time I checked, the URL was:
//
// http://www3.symbian.com/faq.nsf/0/7B5E8CCBA8AEEF7780256E3900506203?OpenDocument
//
#include <smuthdr.h>
#include <smutset.h>
#include <imageconversion.h>

#include <apr_strings.h>


extern "C"
{

int ap_rputd(const TDesC8& aText, request_rec* r)
{
    return ap_rwrite(aText.Ptr(), aText.Length(), r);
}


int ap_rputdL(const TDesC16& aText, request_rec* r)
{
    HBufC8
        *pText8 = HBufC8::NewLC(aText.Length());
    
    pText8->Des().Copy(aText);
    
    int
        rc = ap_rputd(*pText8, r);
    
    CleanupStack::PopAndDestroy(pText8);
    
    return rc;
}


int ap_rputrL(const CRichText& aText, request_rec* r)
{
    HBufC
        *pBuf = HBufC::NewLC(aText.DocumentLength());
    TPtr
        des = pBuf->Des();
    
    aText.Extract(des);
    
    int
        rc = ap_rputdL(des, r);
    
    CleanupStack::PopAndDestroy(pBuf);
    
    return rc;
}

} // extern "C"


namespace
{
    void DoParseQueryString(char* pLine, Apache::QueryArgument* pArgs)
        {
        int
            i = 0;
        
        while (*pLine != 0)
            {
            char
                *pKey   = pLine,
                *pValue = 0,
                *pAnd   = ap_strchr(pKey, '&');
            
            if (pAnd)
                *pAnd = 0;
            
            char
                *pEq = ap_strchr(pKey, '=');
            
            if (pEq)
                {
                *pEq = 0;
                
                pValue = pEq + 1;
                }
            
            pArgs[i].ipKey   = pKey;
            pArgs[i].ipValue = pValue;
            
            if (pAnd)
                pLine = pAnd + 1;
            else if (pValue)
                pLine = pValue + strlen(pValue);
            else 
                pLine = pKey + strlen(pKey);
            
            ++i;
            }
        }
}


namespace Apache
{
    int ParseQueryString(request_rec*    r, 
                         QueryArgument** ppArguments,
                         int*            pnArguments)         
        {
        int
            rc = HTTP_INTERNAL_SERVER_ERROR;
        
        *ppArguments = 0;
        *pnArguments = 0;

        if (r->args)
            {
            char
                *pLine = apr_pstrdup(r->pool, r->args);

            if (pLine)
                {
                // Find out how many arguments there are.
                
                *pnArguments = 1;
                
                char
                    *p = pLine;
                
                while (*p)
                    {
                    if (*p++ == '&')
                        ++*pnArguments;
                    }
                
                apr_size_t
                    size = *pnArguments * sizeof(QueryArgument);
                void
                    *pMem = apr_palloc(r->pool, size);
                
                *ppArguments = static_cast<QueryArgument*>(pMem);
                
                if (*ppArguments)
                    {
                    DoParseQueryString(pLine, *ppArguments);

                    rc = OK;
                    }
                }
            }
        else
            rc = OK;

        return rc;
        }


    TBool GetValue(QueryArgument* pArguments, 
                   int            nArguments, 
                   const char*    pKey,
                   const char**   ppValue)
        {
        TBool
            rc = EFalse;
        QueryArgument
            *i   = pArguments,
            *end = i + nArguments;

        while ((i != end) && !rc)
            {
            if (strcmp(i->ipKey, pKey) == 0)
                {
                if (i->ipValue)
                    {
                    *ppValue = i->ipValue;

                    rc = ETrue;
                    }
                }
            
            ++i;
            }

        return rc;
        }


    namespace
        {
            int ValueOfEncodedChar(char c)
                {
                int
                    n;
                
                if (c >= '0' && c <= '9')
                    n = c - '0';
                else if (c >= 'A' && c <= 'F')
                    n = 10 + c - 'A';
                else if (c >= 'a' && c <= 'f')
                    n = 10 + c - 'a';
                else
                    n = -1;
                
                return n;
                }
        }


    void URLDecode(const char* pFrom, int length, char* pTo)
        {
        const char
            *pEnd = pFrom + length;

        while (pFrom != pEnd)
            {
            char
                c = *pFrom;
                
            switch (c)
                {
                case '+':
                    *pTo++ = ' ';
                    break;

                case '%':
                    {
                    ++pFrom;
                    
                    if (pEnd - pFrom >= 2)
                        {
                        int
                            h = ValueOfEncodedChar(*pFrom++),
                            l = ValueOfEncodedChar(*pFrom);

                        if ((h >= 0) && (l >= 0))
                            {
                            c = h * 16 + l;

                            *pTo++ = c;
                            }
                        }
                    }
                    break;
                    
                default:
                    *pTo++ = c;
                }
            
            ++pFrom;
            }

        *pTo = 0;
        }


    HBufC* str2HBufCLC(const char* pText)
        {
        TPtrC8
            text(reinterpret_cast<const TUint8*>(pText));
        HBufC
            *pBuffer = HBufC::NewLC(text.Length());

        pBuffer->Des().Copy(text);

        return pBuffer;
        }

}


namespace
{
    class CMsvSessionObserver : public CBase,
                                public MMsvSessionObserver
        {
    public:
        static CMsvSessionObserver* NewLC()
            {
            CMsvSessionObserver
                *pThis = new (ELeave) CMsvSessionObserver;
            
            CleanupStack::PushL(pThis);
            
            return pThis;
            }
        
    public:
        void HandleSessionEvent(TMsvSessionEvent aEvent,
                                TAny*            apArg1,
                                TAny*            apArg2,
                                TAny*            apArg3)
            {
            }
        
        
        void HandleSessionEventL(TMsvSessionEvent aEvent,
                                TAny*            apArg1,
                                TAny*            apArg2,
                                TAny*            apArg3)
            {
            }

    private:
        };


    void DoAddSMSToInboxL(CSmsClientMtm* pMtm,
                          CMsvSession*   pSession,
                          const TDesC&   aFrom,
                          const TDesC&   aDescription,
                          const TDesC&   aMessage,
                          TBool          aNew,
                          TBool          aUnread)
        {
        CMsvEntry
            *pEntry = CMsvEntry::NewL(*pSession, 
                                      KMsvGlobalInBoxIndexEntryId,
                                      TMsvSelectionOrdering());
        
        CleanupStack::PushL(pEntry);

        pMtm->SwitchCurrentEntryL(pEntry->EntryId());

        pMtm->CreateMessageL(pEntry->EntryId());
        
        CleanupStack::PopAndDestroy(pEntry);

        CSmsHeader
            &header = pMtm->SmsHeader();
        CSmsMessage
            &message = header.Message();

        CSmsBufferBase
            &buffer = message.Buffer();

        // FIXME: Watch out, what if text is exceedingly long?
        
        buffer.InsertL(0, aMessage);

        TMsvEntry
            entry = pMtm->Entry().Entry();

        entry.SetInPreparation(EFalse);
        entry.SetVisible(ETrue);
        entry.SetNew(aNew);
        entry.SetUnread(aUnread);
        entry.iDetails.Set(aFrom);
        entry.iDescription.Set(aDescription);
        entry.iDate.HomeTime();

        pMtm->Entry().ChangeL(entry);
        
        pMtm->SaveMessageL();
        }
                        
}


namespace Apache
{
    void AddSMSToInboxL(const TDesC& aFrom,
                        const TDesC& aDescription,
                        const TDesC& aMessage,
                        TBool        aNew,
                        TBool        aUnread)
        {
        CMsvSessionObserver
            *pObserver = CMsvSessionObserver::NewLC();
        CMsvSession
            *pSession = CMsvSession::OpenSyncL(*pObserver);
        
        CleanupStack::PushL(pSession);
        
        CClientMtmRegistry
            *pRegistry = CClientMtmRegistry::NewL(*pSession);
        
        CleanupStack::PushL(pRegistry);
        
        CBaseMtm
            *pBaseMtm = pRegistry->NewMtmL(KUidMsgTypeSMS);
        CSmsClientMtm
            *pMtm = static_cast<CSmsClientMtm*>(pBaseMtm);
        
        CleanupStack::PushL(pMtm);
        
        DoAddSMSToInboxL(pMtm, pSession, 
                         aFrom, aDescription, aMessage, aNew, aUnread);
        
        CleanupStack::PopAndDestroy(pMtm);
        CleanupStack::PopAndDestroy(pRegistry);
        CleanupStack::PopAndDestroy(pSession);
        CleanupStack::PopAndDestroy(pObserver);
        }
    }


namespace Apache
{
    MImageConverterCallback::~MImageConverterCallback()
        {
        }


    CImageConverter::CImageConverter() :
        CActive(CActive::EPriorityStandard)
        {
        }

    
    CImageConverter::~CImageConverter()
        {
        delete ipEncoder;
        }
        

    CImageConverter* CImageConverter::NewL()
        {
        CImageConverter
            *pThis = NewLC();
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }
    
    
    CImageConverter* CImageConverter::NewLC()
        {
        CImageConverter
            *pThis = new (ELeave) CImageConverter;
        
        CleanupStack::PushL(pThis);
        
        pThis->ConstructL();
        
        return pThis;
        }
    
    
    void CImageConverter::ConstructL()
        {
        CActiveScheduler::Add(this);
        }


    void CImageConverter::ConvertL(const CFbsBitmap& aBitmap,
                                   const TDesC8&     aMIMEType,
                                   MCallback*        apCallback)
        {
        if (IsActive())
            User::Leave(KErrInUse);
        
        delete ipEncoder;
        
        ipEncoder  = 0;
        ipImage    = 0;
        ipCallback = 0;
        
        ipEncoder = CImageEncoder::DataNewL(ipImage, aMIMEType);
        ipEncoder->Convert(&iStatus, aBitmap);
        
        ipCallback = apCallback;
        
        SetActive();
        }
    


    void CImageConverter::RunL()
        {
        MCallback
            *pCallback = ipCallback;
        HBufC8
            *pImage = ipImage;
        
        ipCallback = 0;
        ipImage    = 0;
        
        switch (iStatus.Int())
            {
            case KErrNone:
                pCallback->ConvertSuccess(this, pImage);
                break;
                
            default:
                pCallback->ConvertError(this, iStatus.Int());
            }
        }
    
    
    void CImageConverter::DoCancel()
        {
        if (ipEncoder)
            ipEncoder->Cancel();
        }
}
