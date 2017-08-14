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

#include "httpds/csettings.h"
#include <f32file.h>
#include <e32const.h>


namespace HttpdS
{
    _LIT8(KTagHttpds,      "httpds");
    _LIT8(KTagHttpd,       "httpd");
    _LIT8(KTagOverride,    "override");
    _LIT8(KTagStackSize,   "stacksize");
    _LIT8(KTagMinHeapSize, "minheapsize");
    _LIT8(KTagMaxHeapSize, "maxheapsize");

    
    NONSHARABLE_CLASS(CSettings::CElement) : public CBase
        {
    public:
        enum TTag
            {
            EUnknown,
            EHttpds,
            EHttpd,
            EOverride,
            EStackSize,
            EMinHeapSize,
            EMaxHeapSize
            };

        ~CElement()
            {
            delete ipParent;
            }
        
        
        TTag Tag() const
            {
            return iTag;
            }
        
        
        virtual void SetContent(const TDesC8&) = 0;

        
        void SetParent(CElement* apParent)
            {
            ipParent = apParent;
            }
        
        
        CElement* Parent()
            {
            return ipParent;
            }
        
        
        void Detach()
            {
            ipParent = 0;
            }
        
    protected:
        CElement(TTag aTag, CElement* apParent)
            : iTag(aTag),
            ipParent(apParent)
            {
            }
        
        void ConstructL()
            {
            }
        
        
        TTag      iTag;
        CElement* ipParent;
        };


    NONSHARABLE_CLASS(CSimpleElement) : public CSettings::CElement
        {
    public:
        static CSimpleElement* NewL(TTag aTag, CElement* apParent = 0)
            {
            CSimpleElement
                *pThis = NewLC(aTag, apParent);
            
            CleanupStack::Pop(pThis);
            return pThis;
            }
        
        
        static CSimpleElement* NewLC(TTag aTag, CElement* apParent = 0)
            {
            CSimpleElement
                *pThis = new (ELeave) CSimpleElement(aTag, apParent);
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();
            return pThis;
            }
        
        
        void SetContent(const TDesC8&)
            {
            }

    private:
        CSimpleElement(TTag aTag, CElement* apParent)
            : CElement(aTag, apParent)
            {
            }
        };


    NONSHARABLE_CLASS(CIntElement) : public CSettings::CElement
        {
    public:
        static CIntElement* NewL(TInt*     apInt,
                                 TTag      aTag, 
                                 CElement* apParent = 0)
            {
            CIntElement
                *pThis = NewLC(apInt, aTag, apParent);
            
            CleanupStack::Pop(pThis);
            return pThis;
            }
            
        
        static CIntElement* NewLC(TInt*     apInt,
                                  TTag      aTag, 
                                  CElement* apParent = 0)
            {
            CIntElement
                *pThis = new (ELeave) CIntElement(apInt, aTag, apParent);
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();
            return pThis;
            }

            
        void SetContent(const TDesC8& aBytes)
            {
            TInt
                length;
            TRadix
                radix;
            TPtrC8
                head = aBytes.Left(2);

            if ((head.Compare(_L8("0x")) == 0) ||
                (head.Compare(_L8("0X")) == 0))
                {
                length = aBytes.Length() - 2;
                radix  = EHex;
                }
            else
                {
                length = aBytes.Length();
                radix  = EDecimal;
                }

            TPtrC8
                bytes = aBytes.Right(length);
            TLex8
                lexer(bytes);
            TUint
                value;
            
            if (lexer.Val(value, radix) == KErrNone)
                iInt = value;
            }
        
    private:
        CIntElement(TInt* apInt, TTag aTag, CElement* apParent)
            : CElement(aTag, apParent),
            iInt(*apInt)
            {
            }
        
        TInt& iInt;
        };
}


namespace HttpdS
{
    CSettings::CSettings()
        {
        }


    CSettings::~CSettings()
        {
        delete ipParser;
        delete ipCurrent;
        }


    CSettings* CSettings::NewL()
        {
        CSettings
            *pThis = NewLC();
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }


    CSettings* CSettings::NewLC()
        {
        CSettings
            *pThis = new (ELeave) CSettings;
        
        CleanupStack::PushL(pThis);
        pThis->ConstructL();
        
        return pThis;
        }

    
    TInt CSettings::ReadL(const char* apPath)
        {
        TPtrC8
            path(reinterpret_cast<const TUint8*>(apPath));
        HBufC8
            *pBuffer = HBufC8::NewLC(path.Length());
        TPtr8
            des = pBuffer->Des();
        
        des.Copy(path);
        
        // Symbian won't have paths with '/' but insists on '\\'.
        
        for (int i = 0; i < des.Length(); ++i)
            if (des[i] == '/')
                des[i] = '\\';

        TInt
            rc = ReadL(des);
            
        CleanupStack::PopAndDestroy(pBuffer);

        return rc;
        }


    TInt CSettings::ReadL(const TDesC8& aPath)
        {
        HBufC
            *pPath = HBufC::NewLC(aPath.Length());

        pPath->Des().Copy(aPath);

        TInt
            rc = ReadL(*pPath);

        CleanupStack::PopAndDestroy(pPath);

        return rc;
        }


    TInt CSettings::ReadL(const TDesC& aPath)
        {
        RFs
            fs;

        User::LeaveIfError(fs.Connect());
            
        CleanupClosePushL(fs);
            
        RFile
            file;
            
        TInt
            rc = file.Open(fs, aPath, EFileShareReadersOnly | EFileRead);

        if (rc == 0)
            {
            CleanupClosePushL(file);

            rc = ReadL(file);

            CleanupStack::PopAndDestroy(); // file
            }

        CleanupStack::PopAndDestroy(); // fs

        return rc;
        }


    TInt CSettings::ReadL(RFile& aFile)
        {
            
        TInt
            size;

        User::LeaveIfError(aFile.Size(size));

        HBufC8
            *pBuffer = HBufC8::NewLC(size);
        TPtr8
            des = pBuffer->Des();
        TInt
            rc = aFile.Read(des);

        if (rc == 0)
            rc = ParseL(des);

        CleanupStack::PopAndDestroy(pBuffer);

        return rc;
        }

        
    TInt CSettings::ParseL(const TDesC8& aSettings)
        {
        Reset();
        
        ipParser->ParseBeginL();
        TRAPD(rc1, ipParser->ParseL(aSettings));
        TRAPD(rc2, ipParser->ParseEndL());

        if (rc1 == KErrNone)
            rc1 = rc2;
        
        if (rc1 != KErrNone)
            Reset();
        
        return rc1;
        }
        

    void CSettings::ConstructL()
        {
        ipParser = Xml::CParser::NewL(_L8("text/xml"), *this);
        }
        
    
    void CSettings::Reset()
        {
        iOverride = false;

        iStackSize   = 0;
        iMinHeapSize = 0;
        iMaxHeapSize = 0;

        delete ipCurrent;
        ipCurrent = 0;
        }


    void 
    CSettings::OnStartDocumentL(const Xml::RDocumentParameters& /*aDocParam*/, 
                                TInt                            aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        }
    
    
    void CSettings::OnEndDocumentL(TInt aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        }
    
    
    void CSettings::OnStartElementL(
        const Xml::RTagInfo&        aElement, 
        const Xml::RAttributeArray& /*aAttributes*/, 
        TInt                        aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        
        const TDesC8
            &element = aElement.LocalName().DesC();
        CElement
            *pElement = 0;
        
        if (element.Compare(KTagHttpds) == 0)
            {
            if (!CurrentElement())
                pElement = CSimpleElement::NewL(CElement::EHttpds);
            }
        else if (element.Compare(KTagHttpd) == 0)
            {
            if (CurrentElement()->Tag() == CElement::EHttpds)
                pElement = CSimpleElement::NewL(CElement::EHttpd);
            }
        else if (element.Compare(KTagOverride) == 0)
            {
            iOverride = true;
            
            if (CurrentElement()->Tag() == CElement::EHttpd)
                pElement = CSimpleElement::NewL(CElement::EOverride);
            }
        else if (element.Compare(KTagStackSize) == 0)
            {
            if (CurrentElement()->Tag() == CElement::EHttpd)
                pElement = CIntElement::NewL(&iStackSize, 
                                             CElement::EStackSize);
            }
        else if (element.Compare(KTagMinHeapSize) == 0)
            {
            if (CurrentElement()->Tag() == CElement::EHttpd)
                pElement = CIntElement::NewL(&iMinHeapSize, 
                                             CElement::EMinHeapSize);
            }
        else if (element.Compare(KTagMaxHeapSize) == 0)
            {
            if (CurrentElement()->Tag() == CElement::EHttpd)
                pElement = CIntElement::NewL(&iMaxHeapSize,
                                             CElement::EMaxHeapSize);
            }
        
        if (!pElement)
            pElement = CSimpleElement::NewL(CElement::EUnknown);
        
        PushElement(pElement);
        }
    
    
    void CSettings::OnEndElementL(const Xml::RTagInfo& /*aElement*/, 
                                  TInt                 aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        PopElement();
        }
    
    
    void CSettings::OnContentL(const TDesC8& aBytes, TInt aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        
        ASSERT(ipCurrent);
        
        ipCurrent->SetContent(aBytes);
        }
    
    
    void CSettings::OnStartPrefixMappingL(const RString& /*aPrefix*/, 
                                          const RString& /*aUri*/, 
                                          TInt           aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        }
    
    
    void CSettings::OnEndPrefixMappingL(const RString& /*aPrefix*/, 
                                        TInt           aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        }
    
    
    void CSettings::OnIgnorableWhiteSpaceL(const TDesC8& /*aBytes*/, 
                                           TInt          aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        }
    
    
    void CSettings::OnSkippedEntityL(const RString& /*aName*/, 
                                     TInt           aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        }
    
    
    void CSettings::OnProcessingInstructionL(const TDesC8& /*aTarget*/, 
                                             const TDesC8& /*aData*/, 
                                             TInt          aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        }
    
    
    void CSettings::OnError(TInt aErrorCode)
        {
        User::LeaveIfError(aErrorCode);
        }
    
    
    TAny* CSettings::GetExtendedInterface(const TInt32 /*aUid*/)
        {
        return 0;
        }


    void CSettings::PushElement(CElement* ipElement)
        {
        ipElement->SetParent(ipCurrent);
        
        ipCurrent = ipElement;
        }
    
    
    void CSettings::PopElement()
        {
        ASSERT(ipCurrent);
        
        CElement
            *pCurrent = ipCurrent;
        
        ipCurrent = pCurrent->Parent();
        
        pCurrent->Detach();
        
        delete pCurrent;
        }
    
    
    CSettings::CElement* CSettings::CurrentElement()
        {
        return ipCurrent;
        }
}
