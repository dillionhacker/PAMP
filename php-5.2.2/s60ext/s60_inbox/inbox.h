/**
 * ====================================================================
 * inbox.h
 * Copyright (c) 2006 Nokia Corporation
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
 * ====================================================================
 */

#include "s60ext_tools.h"
#include <e32std.h>
#include <eikenv.h>
#include <txtrich.h>

#include <mtclbase.h>
#include <msvapi.h>
#include <mtclreg.h>
#include <msvids.h>
#include <smut.h>

const TInt KMessageBodySize = 512;
const TInt KMessageAddressLength = 512;

class CInboxAdapter : public CBase, public MMsvSessionObserver
{
public:
  static CInboxAdapter* NewL();
  static CInboxAdapter* NewLC();
  ~CInboxAdapter();
public: 
  void DeleteMessageL(TMsvId aMessageId);
  void GetMessageTimeL(TMsvId aMessageId, TTime& aTime);
  void GetMessageFlagsL(TMsvId aMessageId, long& flags, long& a, long& b, long& c);
  void GetMessageAddressL(TMsvId aMessageId, TDes& aAddress);
  CArrayFixFlat<TMsvId>* GetMessagesL();  // Passes ownership 
  TBool GetMessageL(TMsvId aMessageId, TDes& aMessage);
private:          // from MMsvSessionObserver
  void HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* aArg3);
private:
  void ConstructL();
  void CompleteConstructL();
private:
  CMsvSession* iSession;          // msg server session
  CBaseMtm* iMtm;                 // Client MTM
  CClientMtmRegistry* iMtmReg;    // Client MTM registry
};
  
