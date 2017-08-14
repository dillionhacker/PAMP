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

#include "mws/cargumentsquerydialog.h"
#include <bautils.h> 
#include "httpdutil/private.h"


namespace
{
    _LIT(KResourcePath, "\\resource\\apps\\httpdutil.rsc");
    _LIT(KArguments,    "Arguments");
}


namespace Mws
{

    EXPORT_C CArgumentsQueryDialog::CArgumentsQueryDialog(TDes&        aText,
                                                          const TTone& aTone)
        : CAknTextQueryDialog(aText, aTone),
          iResource(-1)
        {
        }
                                                          

    EXPORT_C CArgumentsQueryDialog::~CArgumentsQueryDialog()
        {
        if (iResource != -1)
            CEikonEnv::Static()->DeleteResourceFile(iResource);            
        }


    EXPORT_C CArgumentsQueryDialog* 
    CArgumentsQueryDialog::NewL(TDes&        aText,
                                TButtons     aButtons,
                                const TTone& aTone)
        {
        return NewL(KArguments, aText, aButtons, aTone);
        }
    
    
    EXPORT_C CArgumentsQueryDialog* 
    CArgumentsQueryDialog::NewL(const TDesC& aPrompt,
                                TDes&        aText,
                                TButtons     aButtons,
                                const TTone& aTone)
        {
        CArgumentsQueryDialog
            *pThis = NewLC(aPrompt, aText, aButtons, aTone);
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }
    
    
    EXPORT_C CArgumentsQueryDialog*
    CArgumentsQueryDialog::NewLC(TDes&        aText,
                                 TButtons     aButtons,
                                 const TTone& aTone)
        {
        return NewLC(KArguments, aText, aButtons, aTone);
        }
        
    
    EXPORT_C CArgumentsQueryDialog*
    CArgumentsQueryDialog::NewLC(const TDesC& aPrompt,
                                 TDes&        aText,
                                 TButtons     aButtons,
                                 const TTone& aTone)
        {
            CArgumentsQueryDialog
                *pThis = new (ELeave) CArgumentsQueryDialog(aText, aTone);
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL(aPrompt, aButtons);
            
            return pThis;
        }
        
    
    void CArgumentsQueryDialog::UpdateLeftSoftKeyL()
        {
        // An empty string is fine.
        MakeLeftSoftkeyVisible(true);        
        }

        
    void CArgumentsQueryDialog::ConstructL(const TDesC& aPrompt,
                                           TButtons     aButtons)
        {
        CEikonEnv
            *pEnv = CEikonEnv::Static();
        
        TFileName
            path(KResourcePath);

        CompleteWithAppPath(path);

        BaflUtils::NearestLanguageFile(pEnv->FsSession(), path);
        
        iResource = pEnv->AddResourceFileL(path);

        PrepareLC(R_HTTPDUTIL_ARGUMENTS_DIALOG);
        SetPromptL(aPrompt);
        
        if (aButtons == ESaveCancel)
            {
            CEikButtonGroupContainer
                &bgc = ButtonGroupContainer();
            
            bgc.SetCommandSetL(R_HTTPDUTIL_SOFTKEYS_SAVE_CANCEL);
            bgc.MakeVisible(ETrue);
            bgc.DrawDeferred();
            }
        }

} // namespace Mws
