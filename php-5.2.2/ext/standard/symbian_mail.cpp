/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2007 Nokia Corporation                              	  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Mikko Heikelä <mikko.heikela@futurice.com>	                |
   +----------------------------------------------------------------------+
 */
extern "C" {

#include "symbian_mail.h"

// Error reporting etc.
#include <php.h>
// INI access
#include <php_ini.h>
}

// Included Symbian or S60 headers
#include <e32def.h>
#include <e32err.h>
#include <e32cmn.h>
#include <rsendas.h>
#include <rsendasmessage.h>
#include <csendasmessagetypes.h>
#include <mtmuids.h>
#include <msvids.h>
#include <mtclbase.h>
#include <mtclreg.h>

// Included OpenC headers
#include <string.h>
#include <wchar.h>
#include <stdlib.h>

// Account selection code adapted from Paul Todd's Forum Nokia blog:
// http://blogs.forum.nokia.com/view_entry.html?id=462

// Uid of the email technology group.
const TUid KUidMsvTechnologyGroupEmail    = {0x10001671};

// Event observer
class TMessageEventObserver: public MMsvSessionObserver
    {
public:
	// We do not need to handle any events to be able to send mail
    void HandleSessionEventL(TMsvSessionEvent, TAny*, TAny*, TAny*) {};
    };


// Internal error codes
static const TInt KErrorSymbianMailAccountNotSpecified = -100001;
static const TInt KErrorSymbianMailAccountNotFound = -100002;

// Return the TSendAsAccount that corresponds to the account whose name is given in
// "symbian_smtp_account" PHP_INI string.
static TSendAsAccount SelectEmailAccountL() {
	// If a mail account for PHP has not been selected, fail.
	const char* symbian_smtp_account = INI_STR("symbian_smtp_account");
	if( symbian_smtp_account == NULL || strlen(symbian_smtp_account) == 0) {
		User::Leave(KErrorSymbianMailAccountNotSpecified);
	}

	// Open a session to the messaging server
    TMessageEventObserver obs;
    CMsvSession* session = CMsvSession::OpenSyncL(obs);
    CleanupStack::PushL(session);

	// Now get the list of Client MTM's installed on the device
	// by asking the mtm session for the list.
    CClientMtmRegistry* registry = CClientMtmRegistry::NewL(*session);
    CleanupStack::PushL(registry);

	// Enumerate the client mtm's and check its an email technology mtm and it has at least one account.
	// Also remember to ensure the mtm can send a message.
    const TInt count = registry->NumRegisteredMtmDlls();
    for (TInt i=0; i < count; i++)
        {
        const TUid uid = registry->MtmTypeUid(i);

		// Check to see the mtm is an email mtm
        if (registry->TechnologyTypeUid(uid) == KUidMsvTechnologyGroupEmail)
            {
			// Create a client side MTM for the specified UID
			// this is so we can get whether or not the MTM
			// is one that can send a message
            CBaseMtm* mtm = registry->NewMtmL(uid);
            CleanupStack::PushL(mtm);

			// Now check to see if the MTM can send a message
            TInt response = KErrNone;
            if (mtm->QueryCapability(KUidMtmQueryCanSendMsg, response) == KErrNone)
                {
				// The mtm is an email mtm that can send messages.  Search for the requested account.
				// We get the root entry for the MTM and then get all children under the root
				// that match the mtm
                const TMsvSelectionOrdering order(KMsvNoGrouping, EMsvSortByDetails, ETrue);
                CMsvEntry* entry = CMsvEntry::NewL(*session, KMsvRootIndexEntryId, order);
                CleanupStack::PushL(entry);
               
                entry->SetEntryL(KMsvRootIndexEntryId);
                CMsvEntrySelection* selection = entry->ChildrenWithMtmL(uid);
                CleanupStack::PushL(selection);

				// Now we can get the details for the accounts in the mtm and search for the account
				// PHP is configured to use
                const TInt entryCount = selection->Count();
				HBufC8* buffer8 = HBufC8::NewLC(strlen(symbian_smtp_account));
				HBufC* accountNameDesc = HBufC::NewLC(strlen(symbian_smtp_account));
				buffer8->Des().Copy((TUint8*)symbian_smtp_account);
				accountNameDesc->Des().Copy(*buffer8);

                for (TInt j=0; j < entryCount; j++)
                    {
                    entry->SetEntryL(selection->At(j));
                    if(entry->Entry().iDetails.Match(*accountNameDesc) == 0)
                        {
                    	// Matching account found, return it immediately.
                    	TSendAsAccount result = entry->EntryId();
					    CleanupStack::PopAndDestroy(7, session); // account name descriptors, entry, selection,
					                                             // mtm, registry, session
					    return result;
                        }
                    }
                CleanupStack::PopAndDestroy(4, entry); // account name descriptors, entry and selection
                }
           
            CleanupStack::PopAndDestroy(mtm);
            }
        }

    CleanupStack::PopAndDestroy(2, session); // registry, session
	// Ending up here means the requested account was not found among usable email accounts.
	// On successful match the function returns immediately.
	User::Leave(KErrorSymbianMailAccountNotFound);

	// Unreachable
	return TSendAsAccount();
}

// Sends email, leaves on errors.
static void SymbianDoSendMailL(char* to, char* subject, char* message) {
	RSendAs serverConnection;

	// Get email account
	TSendAsAccount account = SelectEmailAccountL();
	
	// Connect to the send as server
	User::LeaveIfError( serverConnection.Connect() );
	CleanupClosePushL( serverConnection );

	// Create the message object
	RSendAsMessage messageObject;
	messageObject.CreateL( serverConnection, account );
	CleanupClosePushL(messageObject);

	// Allocate buffer for headers and message body
	const TInt recipientLength = strlen(to);
	const TInt subjectLength = strlen(subject);
	const TInt messageLength = strlen(message);
	const TInt descriptorLength = Max(recipientLength,
	                                   Max(subjectLength, messageLength));
	HBufC8* buffer8 = HBufC8::NewLC(descriptorLength);
	HBufC* buffer = HBufC::NewLC(descriptorLength);

	// Set the recipient
	buffer8->Des().Copy((TUint8*)to);
	buffer->Des().Copy(*buffer8);
	messageObject.AddRecipientL(*buffer, RSendAsMessage::ESendAsRecipientTo);

	// Set the message subject
	buffer8->Des().Copy((TUint8*)subject);
	buffer->Des().Copy(*buffer8);
	messageObject.SetSubjectL(*buffer);

	// Set the message body
	buffer8->Des().Copy((TUint8*)message);
	buffer->Des().Copy(*buffer8);
	messageObject.SetBodyTextL(*buffer);
	
	// Clean up the buffers for the message parts
	CleanupStack::PopAndDestroy(2, buffer8);

	// Send the message synchronously (or leave)
	messageObject.SendMessageAndCloseL();
	CleanupStack::Pop(&messageObject);
	
	// Clean up
	CleanupStack::PopAndDestroy(&serverConnection);
}

extern "C" {


// Symbian version of php_mail.  headers and extra_cmd are ignored.
int symbian_php_mail(char *to, char *subject, char *message, char *headers, char *extra_cmd)
    {
	TRAPD(err, SymbianDoSendMailL(to, subject, message));
	
	TSRMLS_FETCH();
	
	switch( err )
	    {
		case KErrNone:
			return 1;
	    case KErrorSymbianMailAccountNotSpecified:
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Symbian SMTP account not specified.  Add symbian_smtp_account to INI.");
			return 0;
	    case KErrorSymbianMailAccountNotFound:
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Symbian SMTP account \"%s\" not found.", INI_STR("symbian_smtp_account"));
			return 0;
		default:
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error sending email, Symbian leave code: %d", err);
			return 0;
		}
    }

}

