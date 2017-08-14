#ifndef mws_cargumentsquerydialog_h
#define mws_cargumentsquerydialog_h
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
#include <aknquerydialog.h> 


namespace Mws
{

    class CArgumentsQueryDialog : private CAknTextQueryDialog
        {
    public:
        enum TButtons
            {
            EOkCancel,
            ESaveCancel
            };
        
        IMPORT_C ~CArgumentsQueryDialog();
        
        IMPORT_C static CArgumentsQueryDialog* NewL (TDes&          aArguments,
                                                     TButtons       aButtons,
                                                     const TTone& = ENoTone);
        IMPORT_C static CArgumentsQueryDialog* NewL (const TDesC&   aPrompt,
                                                     TDes&          aArguments,
                                                     TButtons       aButtons,
                                                     const TTone& = ENoTone);

        IMPORT_C static CArgumentsQueryDialog* NewLC(TDes&          aArguments,
                                                     TButtons       aButtons,
                                                     const TTone& = ENoTone);
        IMPORT_C static CArgumentsQueryDialog* NewLC(const TDesC&   aPrompt,
                                                     TDes&          aArguments,
                                                     TButtons       aButtons,
                                                     const TTone& = ENoTone);
        
        CAknTextQueryDialog::RunLD;
        
    protected:
        IMPORT_C CArgumentsQueryDialog(TDes&        aArguments,
                                       const TTone& aTone);

        IMPORT_C void UpdateLeftSoftKeyL();
        
        IMPORT_C void ConstructL(const TDesC& aPrompt,
                                 TButtons     aButtons);

    protected:
        TInt iResource;
        };

} // namespace Mws

#endif
