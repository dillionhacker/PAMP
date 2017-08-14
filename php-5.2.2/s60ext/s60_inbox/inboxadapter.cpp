/**
 * ====================================================================
 * inboxadapter.cpp
 * Copyright (c) 2005 Nokia Corporation
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
 
#include "inbox.h"

//////////////////////////////////////////////

CInboxAdapter* CInboxAdapter::NewL()
{
	CInboxAdapter* self = NewLC();
	CleanupStack::Pop(self); 
	return self;
}

CInboxAdapter* CInboxAdapter::NewLC()
{
	CInboxAdapter* self = new (ELeave) CInboxAdapter();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

void CInboxAdapter::ConstructL()
{
	iSession = CMsvSession::OpenSyncL(*this); // new session is opened synchronously  
	CompleteConstructL();       // Construct the mtm registry
}

void CInboxAdapter::CompleteConstructL()
{
	// We get a MtmClientRegistry from our session
	// this registry is used to instantiate new mtms.
	iMtmReg = CClientMtmRegistry::NewL(*iSession);
	iMtm = iMtmReg->NewMtmL(KUidMsgTypeSMS);        // create new SMS MTM
}

CInboxAdapter::~CInboxAdapter()
{
	delete iMtm;
	delete iMtmReg;
	delete iSession; // must be last to be deleted
}

void CInboxAdapter::DeleteMessageL(TMsvId aMessageId)
{
	iMtm->SwitchCurrentEntryL(aMessageId);
	iMtm->LoadMessageL();
	TMsvId parent = iMtm->Entry().Entry().Parent();
	iMtm->SwitchCurrentEntryL(parent);
	iMtm->Entry().DeleteL(aMessageId);
}

void CInboxAdapter::GetMessageTimeL(TMsvId aMessageId, TTime& aTime)
{
	iMtm->SwitchCurrentEntryL(aMessageId);	
	iMtm->LoadMessageL();
	aTime = (iMtm->Entry().Entry().iDate);
}

void CInboxAdapter::GetMessageFlagsL(TMsvId aMessageId, long& flags, long& a, long& b, long& c)
{
	iMtm->SwitchCurrentEntryL(aMessageId);	
	iMtm->LoadMessageL();
	const TMsvEntry& e = iMtm->Entry().Entry();
	flags = 0;
	/*
	flags |= e.Attachment()     ? 0x0001 : 0;
	flags |= e.Complete()       ? 0x0002 : 0;
	flags |= e.Connected()      ? 0x0004 : 0;
	flags |= e.Deleted()        ? 0x0008 : 0;
	flags |= e.Failed()         ? 0x0010 : 0;
	flags |= e.InPreparation()  ? 0x0020 : 0;
	flags |= e.MultipleRecipients() ? 0x0040 : 0;
	flags |= e.New()            ? 0x0080 : 0;
	flags |= e.OffPeak()        ? 0x0100 : 0;
	flags |= e.Operation()      ? 0x0200 : 0;
	flags |= e.Owner()          ? 0x0400 : 0;
	flags |= e.Scheduled()      ? 0x1000 : 0;
	flags |= e.StandardFolder() ? 0x2000 : 0;
*/
	flags |= e.ReadOnly()       ? 0x0800 : 0;
	a = 12345;//e.iMtmData1;
	b = e.iMtmData2;
	c = e.iMtmData3;
	//flags |= e.Unread()         ? 0x4000 : 0;
	//flags |= e.Visible()        ? 0x8000 : 0;
}

void CInboxAdapter::GetMessageAddressL(TMsvId aMessageId, TDes& aAddress)
{
	iMtm->SwitchCurrentEntryL(aMessageId);	
	iMtm->LoadMessageL();
	TPtrC address = iMtm->Entry().Entry().iDetails;
	TInt length = address.Length();
	
	// Check length because address is read to a limited size TBuf
	if (length >= KMessageAddressLength) 
	{
		aAddress.Append(address.Left(KMessageAddressLength - 1));
	}
	else 
	{
		aAddress.Append(address.Left(length));
	}  
}  

CArrayFixFlat<TMsvId>* CInboxAdapter::GetMessagesL()
{
	// XXX add here e.g. Sent box and e-mail type also:
	CMsvEntry* parentEntry=NULL;
	CMsvEntrySelection* entries=NULL;
	parentEntry = CMsvEntry::NewL(*iSession, 
	                            KMsvGlobalInBoxIndexEntryId, 
	                            TMsvSelectionOrdering()); //pointer to Messages Inbox
	entries = parentEntry->ChildrenWithMtmL(KUidMsgTypeSMS); // select sms entries
	delete parentEntry;
	return (CArrayFixFlat<TMsvId>*)entries;
}

TBool CInboxAdapter::GetMessageL(TMsvId aMessageId, TDes& aMessage)
{
	iMtm->SwitchCurrentEntryL(aMessageId);
	
	if (iMtm->Entry().HasStoreL()) 
	{
		// SMS message is stored inside Messaging store
		CMsvStore* store = iMtm->Entry().ReadStoreL();
		CleanupStack::PushL(store);
		
		if (store->HasBodyTextL())
		{
			CParaFormatLayer* pfl = CParaFormatLayer::NewL();
			CleanupStack::PushL(pfl);
			CCharFormatLayer* cfl = CCharFormatLayer::NewL();
			CleanupStack::PushL(cfl);

			CRichText* richText = CRichText::NewL(pfl, cfl);
				//CEikonEnv::Static()->SystemParaFormatLayerL(),
				//CEikonEnv::Static()->SystemCharFormatLayerL()); //XXX crash!
			richText->Reset();
			CleanupStack::PushL(richText);
			
			// Get the SMS body text.
			store->RestoreBodyTextL(*richText);
			const TInt length = richText->DocumentLength();
			TBuf<KMessageBodySize> message;
			
			// Check length because message is read to limited size TBuf
			if (length >= KMessageBodySize) 
			{
				message.Append(richText->Read(0, KMessageBodySize -1));
			}
			else 
			{
				message.Append(richText->Read(0, length));
			}
			
			aMessage.Append(message);
			CleanupStack::PopAndDestroy(richText);
			CleanupStack::PopAndDestroy(cfl);
			CleanupStack::PopAndDestroy(pfl);
		}
		
		CleanupStack::PopAndDestroy(store);
		
	}
	else
	{
		return EFalse;
	}
	
	return ETrue;
}


void CInboxAdapter::HandleSessionEventL(TMsvSessionEvent , TAny* , TAny* , TAny* )
{
}
