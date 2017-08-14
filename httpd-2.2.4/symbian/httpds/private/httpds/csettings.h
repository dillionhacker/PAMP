#ifndef httpds_csettings_h
#define httpds_csettings_h
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

#include "httpds/private.h"
#include <xml/parser.h>
#include <xml/contenthandler.h>


namespace HttpdS
{
    NONSHARABLE_CLASS(CSettings): public  CBase,
                                  private Xml::MContentHandler
        {
    public:
        ~CSettings();
        
        static CSettings* NewL();
        static CSettings* NewLC();
        
        TInt ReadL(const char* apPath);
        TInt ReadL(const TDesC8& aPath);
        TInt ReadL(const TDesC& aPath);
        TInt ReadL(RFile& aFile);
        
        TInt ParseL(const TDesC8& aSettings);
        

        TBool Override() const { return iOverride; }
        
        TInt  StackSize() const { return iStackSize; }
        
        TInt  MinHeapSize() const { return iMinHeapSize; }

        TInt  MaxHeapSize() const { return iMaxHeapSize; }

    private:
        // Xml::MContentHandler
        void OnStartDocumentL(const Xml::RDocumentParameters& aDocParam, 
                              TInt                            aErrorCode);

        void OnEndDocumentL(TInt aErrorCode);

        void OnStartElementL(const Xml::RTagInfo&        aElement, 
                             const Xml::RAttributeArray& aAttributes, 
                             TInt                        aErrorCode);

        void OnEndElementL(const Xml::RTagInfo& aElement, 
                           TInt                 aErrorCode);

        void OnContentL(const TDesC8& aBytes, TInt aErrorCode);

        void OnStartPrefixMappingL(const RString& aPrefix, 
                                   const RString& aUri, 
                                   TInt           aErrorCode);

        void OnEndPrefixMappingL(const RString& aPrefix, 
                                 TInt           aErrorCode);

        void OnIgnorableWhiteSpaceL(const TDesC8& aBytes, 
                                    TInt          aErrorCode);

        void OnSkippedEntityL(const RString& aName, 
                              TInt           aErrorCode);

        void OnProcessingInstructionL(const TDesC8& aTarget, 
                                      const TDesC8& aData, 
                                      TInt          aErrorCode);

        void OnError(TInt aErrorCode);

        TAny* GetExtendedInterface(const TInt32 aUid);
        
    public:
        class CElement;

    private:
        CSettings();

        void ConstructL();

        void Reset();

        void PushElement(CElement* apElement);
        void PopElement();
        
        CElement* CurrentElement();
        
    private:
        Xml::CParser* ipParser;
        CElement*     ipCurrent;
        TBool         iOverride;
        TInt          iStackSize;
        TInt          iMinHeapSize;
        TInt          iMaxHeapSize;
        };
}

#endif
