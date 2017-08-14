/**
 * ====================================================================
 *  PHP API to inbox/SMS handling, partly modified from the corresponding
 *  Python API. Allows manipulating of host phone's SMS inbox.
 * --------------------------------------------------------------------
 *
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

/**
//EXAMPLE

<pre><?php

echo "\n\nALL MESSAGES:\n";
echo "=========================================\n";
foreach (s60_inbox_get_messages() as $msg) {
	echo "  SENDER: ".$msg->sender()."\n";
	echo "  CONTENT: ".$msg->content()."\n";
	echo "  TIME: ".strftime ("%a, %d %b %Y at %H:%M:%S", $msg->time())."\n";
	echo "=========================================\n";
}

//s60_inbox_send_message("hello there!", "mr.nobody");

?></pre>

*/

#include <SMSCLNT.h>
#include <SMUTHDR.h>
#include <smutset.h>
#include "inbox.h"
#include "s60ext_tools.h"



extern "C" {

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h" 
#include "inboxmodule.h"

BEGIN_MODULE_FUNCTIONS(s60_inbox)
	PHP_FE(s60_inbox_send_message, NULL)
	PHP_FE(s60_inbox_get_messages, NULL)
END_MODULE_FUNCTIONS()

DEFINE_MODULE(s60_inbox, "1.0", PHP_MINIT(s60_inbox), NULL, NULL, NULL)

static CInboxAdapter * g_inbox;
static int g_inbox_refcount;

static void inbox_incref() {
	if (g_inbox_refcount == 0) {
		TRAPD(error, g_inbox = CInboxAdapter::NewL());
		if (error != KErrNone) {
			g_inbox = 0;
			g_inbox_refcount = 0;
		} else {
			g_inbox_refcount = 1;
		}
	} else {
		g_inbox_refcount++;	
	}
}

static void inbox_decref() {
	g_inbox_refcount--;	
	if (g_inbox_refcount <= 0) {
		g_inbox_refcount = 0;
		if (g_inbox) {
			delete g_inbox;
			g_inbox = 0;
		}
	}
}

CLASS_REGISTER_FUNCTION(s60_inbox_Message);

PHP_MINIT_FUNCTION(s60_inbox)
{
	g_inbox_refcount = 0;
	g_inbox = 0;

	CLASS_REGISTER_CALL(s60_inbox_Message);
	return SUCCESS;
}


/**********************************************************************
 * FREE FUNCTIONS
 **********************************************************************/


namespace
{

	/*
	 * Functions/classes for sending SMS messages to the host phone's
	 * inbox. Copied from mod_inbox
	 */	
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
	

    void doAddSMSToInboxL(CSmsClientMtm* pMtm,
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

        CSmsBufferBase
            &buffer = pMtm->SmsHeader().Message().Buffer();

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
#ifdef EKA2
        entry.iDate.UniversalTime();
#else
        entry.iDate.HomeTime();
#endif

        pMtm->Entry().ChangeL(entry);
        
        pMtm->SaveMessageL();
        }
                        


    void addSMSToInboxL(const TDesC& aFrom,
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
        
        doAddSMSToInboxL(pMtm, pSession, 
                         aFrom, aDescription, aMessage, aNew, aUnread);
        
        CleanupStack::PopAndDestroy(pMtm);
        CleanupStack::PopAndDestroy(pRegistry);
        CleanupStack::PopAndDestroy(pSession);
        CleanupStack::PopAndDestroy(pObserver);
        }
}

 

/**
 * s60_inbox_send_message(<string> message, <string> sender)
 * Send SMS message to host phone's inbox.
 */		
PHP_FUNCTION(s60_inbox_send_message)
{
	char *message, *sender;
	int message_len, sender_len;
	GET_FUNCTION_ARGS_4("ss", &message, &message_len, &sender, &sender_len);
	TRAPD(error, {
		HBufC *pMessage = convert_c_string_to_unicodeLC(message, message_len);
		HBufC *pSender = convert_c_string_to_unicodeLC(sender, sender_len);
		addSMSToInboxL(*pSender, *pMessage, *pMessage, true, true);
		CleanupStack::PopAndDestroy(pSender);
		CleanupStack::PopAndDestroy(pMessage);
	});
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}





/**********************************************************************
 * Class s60_inbox_Message
 * This class represents a single message in the phone's inbox.
 **********************************************************************/
 
BEGIN_CLASS_STRUCT(s60_inbox_Message)
	TMsvId message_id;
END_CLASS_STRUCT()

DEFINE_CLASS_DATA(s60_inbox_Message)

BEGIN_CLASS_FUNCTIONS(s60_inbox_Message)
	PHP_ME(s60_inbox_Message, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_inbox_Message, content, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_inbox_Message, sender, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_inbox_Message, time, NULL, ZEND_ACC_PUBLIC)
	//PHP_ME(s60_inbox_Message, flags, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_inbox_Message, remove, NULL, ZEND_ACC_PUBLIC)
END_CLASS_FUNCTIONS()

TSRMLS_FETCH();

DEFINE_OBJECT_FREE_FUNCTION(s60_inbox_Message, inbox_decref())
DEFINE_OBJECT_NEW_FUNCTION(s60_inbox_Message, inbox_incref())
DEFINE_CLASS_REGISTER_FUNCTION(s60_inbox_Message)


PHP_METHOD(s60_inbox_Message, __construct)
{
	long mid;
	GET_METHOD_ARGS_1(s60_inbox_Message, "Ol", &mid);
	self->message_id = mid;
}

#define RETURN_IF_NOTFOUND_ERROR(_eRRor_) { if ((_eRRor_) == KErrNotFound) { RETURN_ERROR("Message not found"); } }
#define ASSERT_INBOX { if (!g_inbox) { RETURN_ERROR("Inbox not open"); } }


/**
 * <array of s60_inbox_Message> s60_inbox_get_messages()
 *   Return an array of host phone's inbox SMS messages,
 *   ordered so that newest message is first. The returned
 *   array contains objects of type s60_inbox_Message
 */
PHP_FUNCTION(s60_inbox_get_messages)
{
	GET_FUNCTION_ARGS_0();
	inbox_incref();
	ASSERT_INBOX;
	
	CArrayFixFlat<TMsvId>* sms_list=NULL;
	TRAPD(error, sms_list = g_inbox->GetMessagesL())
	if (error != KErrNone) {
		inbox_decref();
		RETURN_SYMBIAN_ERROR(error);
	}
	
	array_init(return_value);
	for (int i = (sms_list->Count() - 1); i >= 0; i--) {
		zval *new_object;
		ALLOC_INIT_ZVAL(new_object);
		if (object_init_ex(new_object, CLASS_ENTRY(s60_inbox_Message)) == SUCCESS) {
			GET_STRUCT(new_object, s60_inbox_Message)->message_id = sms_list->At(i);
			add_next_index_zval(return_value, new_object);
		} else {
			zval_ptr_dtor(&new_object); // new_object wasn't added to return_value
			inbox_decref(); // inbox isn't used anymore
			delete sms_list; // and sms_list must be freed
			RETURN_ERROR("Could not allocate all messages");
		}
	}
	
	inbox_decref();
	
	delete sms_list;
}

/**
 * <string> s60_inbox_Message::content()
 * Get message content.
 */
PHP_METHOD(s60_inbox_Message, content)
{
	GET_METHOD_ARGS_0(s60_inbox_Message);
	ASSERT_INBOX;
	
	//XXX KMessageBodySize is too short. Messages can be longer than 256 chars
	TBuf<KMessageBodySize> content;
	TRAPD(error, g_inbox->GetMessageL(self->message_id, content))
	RETURN_IF_NOTFOUND_ERROR(error);
	RETURN_IF_SYMBIAN_ERROR(error);
	
	error = ZVAL_SYMBIAN_STRING(return_value, content);
	RETURN_IF_SYMBIAN_ERROR(error);
}


/**
 * <float> s60_inbox_Message::time()
 * Get message time of arrival.
 */
PHP_METHOD(s60_inbox_Message, time)
{
	GET_METHOD_ARGS_0(s60_inbox_Message);
	ASSERT_INBOX;
	
	TTime arrival;
	TRAPD(error, g_inbox->GetMessageTimeL(self->message_id, arrival))
	RETURN_IF_NOTFOUND_ERROR(error);
	RETURN_IF_SYMBIAN_ERROR(error);
#ifdef EKA2
	RETURN_DOUBLE(convert_utc_time_to_utc_real(arrival));
#else
	RETURN_DOUBLE(convert_time_to_utc_real(arrival));
#endif
}


/**
 * <int> s60_inbox_Message::flags()
 * Get message flags
PHP_METHOD(s60_inbox_Message, flags)
{
	GET_METHOD_ARGS_0(s60_inbox_Message);
	ASSERT_INBOX;
	
	long flags = 0, a = 0, b = 0, c = 0;
	TRAPD(error, g_inbox->GetMessageFlagsL(self->message_id, flags, a, b, c))
	RETURN_IF_NOTFOUND_ERROR(error);
	RETURN_IF_SYMBIAN_ERROR(error);
	//RETURN_LONG(flags);
	array_init(return_value);
	add_next_index_long(return_value, flags);
	TTime t;
	t.HomeTime();
	add_next_index_double(return_value, convert_utc_time_to_utc_real(t));
	t.UniversalTime();
	add_next_index_double(return_value, convert_utc_time_to_utc_real(t));
}
 */

/**
 * <string> s60_inbox_Message::sender()
 * Get message sender.
 */
PHP_METHOD(s60_inbox_Message, sender)
{
	GET_METHOD_ARGS_0(s60_inbox_Message);
	ASSERT_INBOX;
	
	TBuf<KMessageAddressLength> address;
	TRAPD(error, g_inbox->GetMessageAddressL(self->message_id, address))
	RETURN_IF_NOTFOUND_ERROR(error);
	RETURN_IF_SYMBIAN_ERROR(error);

	error = ZVAL_SYMBIAN_STRING(return_value, address);
	RETURN_IF_SYMBIAN_ERROR(error);
}


/**
 * s60_inbox_Message::remove()
 * Delete message.
 */
PHP_METHOD(s60_inbox_Message, remove)
{
	GET_METHOD_ARGS_0(s60_inbox_Message);
	ASSERT_INBOX;
	
	TRAPD(error, g_inbox->DeleteMessageL(self->message_id))
	RETURN_IF_NOTFOUND_ERROR(error);
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}



} /* extern "C" */

