/**
 * ====================================================================
 *  PHP API to contacts handling, modified from the corresponding
 *  Python API. Allows reading and modifying of contact information.
 *
 *  Using module will crash (panic) on S60 version 3.x if it hasn't been
 *  installed with proper capabilities: WriteDeviceData and ReadDeviceData.
 *
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

// Prints some field type information (s60_contacts_FieldType)
function print_field_type($ft) {
	printf("%-20s ID:%3d   L:%3d   S:%3d   M:%3d | #:%3d   @:%3d\n",
	       $ft->name(), $ft->id(), $ft->location(), $ft->storage_type(),
	       $ft->multi(), $ft->is_phone_number(), $ft->is_email());
}

// Prints all field information (s60_contacts_Field)
function print_field($f) {
	echo "    LABEL: ".$f->label()."\n";
	echo "      VALUE: ".$f->value()."\n";
	echo "      TYPE: "; print_field_type($f->type());
}

// Prints all contact information (s60_contacts_Contact)
function print_contact($c) {
	echo " ID: ".$c->id()."\n";
	echo " TITLE: ".$c->title()."\n";
	echo " MODIFIED: ".strftime ("%a, %d %b %Y at %H:%M:%S", $c->last_modified())."\n";
	echo " FIELDS:\n";
	foreach ($c->fields() as $f) {
		print_field($f);
	}
}

function test() {
	var_dump(S60_CONTACTS_LAST_NAME);
	var_dump(S60_CONTACTS_FIRST_NAME);
	$db = s60_contacts_open();
	
	// create new contact and print out its info
	$c = $db->add_contact();
	$f = $c->add_field(S60_CONTACTS_FIRST_NAME);
	$f->set_value("frst");
	$f = $c->add_field(S60_CONTACTS_LAST_NAME);
	$f->set_value("lst");
	
	echo "TEST CONTACT:\n";
	echo "=========================================\n";
	print_contact($c);
	
	// add the contact to database:
	//$c->commit();
	
	// remove the contact:
	//$c->remove();
	
	// print out all your contacts
	echo "\n\nALL CONTACTS:\n";
	echo "=========================================\n";
	foreach ($db->contacts() as $c) {
		print_contact($c);
		echo "--------------------------------\n";
	}
	
	// print out all available field types
	echo "\n\nFIELD TYPES:\n";
	echo "=========================================\n";
	$types = $db->field_types();
	var_dump($db->close());
	foreach ($types as $ft) {
		print_field_type($ft);
	}
}

test();

?></pre>

*/

/**
//CONSTANTS

    // Field id:s (see s60_contacts_FieldType::id())
	REGISTER_LONG_CONSTANT("S60_CONTACTS_NONE", EPbkFieldIdNone, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_LAST_NAME", EPbkFieldIdLastName, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_FIRST_NAME", EPbkFieldIdFirstName, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PHONE_GENERAL", EPbkFieldIdPhoneNumberGeneral, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PHONE_STANDARD", EPbkFieldIdPhoneNumberStandard, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PHONE_HOME", EPbkFieldIdPhoneNumberHome, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PHONE_WORK", EPbkFieldIdPhoneNumberWork, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PHONE_MOBILE", EPbkFieldIdPhoneNumberMobile, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_FAX", EPbkFieldIdFaxNumber, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PAGER", EPbkFieldIdPagerNumber, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_EMAIL", EPbkFieldIdEmailAddress, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_POSTAL_ADDRESS", EPbkFieldIdPostalAddress, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_URL", EPbkFieldIdURL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_JOB_TITLE", EPbkFieldIdJobTitle, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_COMPANY_NAME", EPbkFieldIdCompanyName, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_COMPANY_ADDRESS", EPbkFieldIdCompanyAddress, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_DTMF_STRING", EPbkFieldIdDTMFString, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_DATE", EPbkFieldIdDate, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_NOTE", EPbkFieldIdNote, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PO_BOX", EPbkFieldIdPOBox, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_EXTENDED_ADDDRESS", EPbkFieldIdExtendedAddress, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STREET_ADDRESS", EPbkFieldIdStreetAddress, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_POSTAL_CODE", EPbkFieldIdPostalCode, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_CITY", EPbkFieldIdCity, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STATE", EPbkFieldIdState, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_COUNTRY", EPbkFieldIdCountry, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_WVID", EPbkFieldIdWVID, CONST_CS | CONST_PERSISTENT);
	
    // Location id:s (see s60_contacts_FieldType::location())
	REGISTER_LONG_CONSTANT("S60_CONTACTS_LOCATION_NONE", EPbkFieldLocationNone, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_LOCATION_HOME", EPbkFieldLocationHome, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_LOCATION_WORK", EPbkFieldLocationWork, CONST_CS | CONST_PERSISTENT);
   
    // Storage types (see s60_contacts_FieldType::storage_type())
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STORAGE_TEXT", KStorageTypeText, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STORAGE_STORE", KStorageTypeStore, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STORAGE_CONTACT", KStorageTypeContactItemId, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STORAGE_TIME", KStorageTypeDateTime, CONST_CS | CONST_PERSISTENT);
   
    // Field type multiplicity (see s60_contacts_FieldType::multi())
	REGISTER_LONG_CONSTANT("S60_CONTACTS_MULTI_ONE", EPbkFieldMultiplicityOne, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_MULTI_MANY", EPbkFieldMultiplicityMany, CONST_CS | CONST_PERSISTENT);

*/

#include <cntdb.h>
#include <cntitem.h>
#include <cntfldst.h>
#include <utf.h>

#include <cpbkcontactengine.h>
#include <CPbkContactIter.h> 
#include <CPbkContactItem.h> 
#include <CPbkFieldsInfo.h> 
#include <s32mem.h> 

#include "s60ext_tools.h"

extern "C" {

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h" 
#include "contacts.h"

BEGIN_MODULE_FUNCTIONS(s60_contacts)
	PHP_FE(s60_contacts_open, NULL)
END_MODULE_FUNCTIONS()

DEFINE_MODULE(s60_contacts, "1.0", PHP_MINIT(s60_contacts), NULL, NULL, NULL)

CLASS_REGISTER_FUNCTION(s60_contacts_Database);
CLASS_REGISTER_FUNCTION(s60_contacts_FieldType);
CLASS_REGISTER_FUNCTION(s60_contacts_Contact);
CLASS_REGISTER_FUNCTION(s60_contacts_Field);

PHP_MINIT_FUNCTION(s60_contacts)
{
    // Field id:s.
	REGISTER_LONG_CONSTANT("S60_CONTACTS_NONE", EPbkFieldIdNone, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_LAST_NAME", EPbkFieldIdLastName, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_FIRST_NAME", EPbkFieldIdFirstName, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PHONE_GENERAL", EPbkFieldIdPhoneNumberGeneral, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PHONE_STANDARD", EPbkFieldIdPhoneNumberStandard, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PHONE_HOME", EPbkFieldIdPhoneNumberHome, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PHONE_WORK", EPbkFieldIdPhoneNumberWork, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PHONE_MOBILE", EPbkFieldIdPhoneNumberMobile, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_FAX", EPbkFieldIdFaxNumber, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PAGER", EPbkFieldIdPagerNumber, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_EMAIL", EPbkFieldIdEmailAddress, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_POSTAL_ADDRESS", EPbkFieldIdPostalAddress, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_URL", EPbkFieldIdURL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_JOB_TITLE", EPbkFieldIdJobTitle, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_COMPANY_NAME", EPbkFieldIdCompanyName, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_COMPANY_ADDRESS", EPbkFieldIdCompanyAddress, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_DTMF_STRING", EPbkFieldIdDTMFString, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_DATE", EPbkFieldIdDate, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_NOTE", EPbkFieldIdNote, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_PO_BOX", EPbkFieldIdPOBox, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_EXTENDED_ADDDRESS", EPbkFieldIdExtendedAddress, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STREET_ADDRESS", EPbkFieldIdStreetAddress, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_POSTAL_CODE", EPbkFieldIdPostalCode, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_CITY", EPbkFieldIdCity, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STATE", EPbkFieldIdState, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_COUNTRY", EPbkFieldIdCountry, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_WVID", EPbkFieldIdWVID, CONST_CS | CONST_PERSISTENT);
	
    // Location id:s.
	REGISTER_LONG_CONSTANT("S60_CONTACTS_LOCATION_NONE", EPbkFieldLocationNone, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_LOCATION_HOME", EPbkFieldLocationHome, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_LOCATION_WORK", EPbkFieldLocationWork, CONST_CS | CONST_PERSISTENT);
   
    // Storage types.
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STORAGE_TEXT", KStorageTypeText, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STORAGE_STORE", KStorageTypeStore, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STORAGE_CONTACT", KStorageTypeContactItemId, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_STORAGE_TIME", KStorageTypeDateTime, CONST_CS | CONST_PERSISTENT);
   
    // Field type multiplicity.
	REGISTER_LONG_CONSTANT("S60_CONTACTS_MULTI_ONE", EPbkFieldMultiplicityOne, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("S60_CONTACTS_MULTI_MANY", EPbkFieldMultiplicityMany, CONST_CS | CONST_PERSISTENT);
	
	CLASS_REGISTER_CALL(s60_contacts_Database);
	CLASS_REGISTER_CALL(s60_contacts_FieldType);
	CLASS_REGISTER_CALL(s60_contacts_Contact);
	CLASS_REGISTER_CALL(s60_contacts_Field);
	return SUCCESS;
}

/**********************************************************************
 * Class s60_contacts_Database
 * This class represents a connection to a contacts database.
 **********************************************************************/


class ContactsDbHandle : public Refcounted {
	~ContactsDbHandle() {
	    delete contactEngine;
	    fileServer.Close();
	}
	
public:
	ContactsDbHandle() : contactEngine(0) {}

	CPbkContactEngine* contactEngine;
	RFs fileServer;
}; 

BEGIN_CLASS_STRUCT(s60_contacts_Database)
	ContactsDbHandle* dbHandle; //if 0, db is closed
END_CLASS_STRUCT()

DEFINE_CLASS_DATA(s60_contacts_Database)

BEGIN_CLASS_FUNCTIONS(s60_contacts_Database)
	PHP_ME(s60_contacts_Database, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Database, open, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Database, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Database, field_types, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Database, add_contact, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Database, get_contact, NULL, ZEND_ACC_PUBLIC)
	//PHP_ME(s60_contacts_Database, find, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Database, contacts, NULL, ZEND_ACC_PUBLIC)
END_CLASS_FUNCTIONS()

DEFINE_OBJECT_FREE_FUNCTION(s60_contacts_Database, {
	SAFE_CLEAR_REFERENCE(self->dbHandle);
})
DEFINE_OBJECT_NEW_FUNCTION(s60_contacts_Database, {
	self->dbHandle = 0;
})
	


DEFINE_CLASS_REGISTER_FUNCTION(s60_contacts_Database)

/**
 * s60_contacts_Database::__construct()
 * Constructs a closed contact database object.
 */
PHP_METHOD(s60_contacts_Database, __construct)
{
	GET_METHOD_ARGS_0(s60_contacts_Database);
}


static char parse_flag(char* flag_str, int flag_str_len)
{
	if (flag_str && flag_str_len > 1) {
		return 0;
	} else if (flag_str && flag_str_len == 1) {
		if (flag_str[0] != 'n' && flag_str[0] != 'c' && flag_str[0] != 'e') {
			return 0;
		}
		return flag_str[0];
	}
	return 'e';	
}


static void s60_contacts_do_open(
	CLASS_STRUCT(s60_contacts_Database)* self,
	char* filename, int filename_len,
	char* flag_str, int flag_str_len,
	zval* return_value)
{
	char flag = parse_flag(flag_str, flag_str_len);
	if (!flag) RETURN_ERROR("Illegal flag (use 'n', 'c', 'e')");
	
	
	// Make sure the database is closed before we try to open a new one.
	SAFE_CLEAR_REFERENCE(self->dbHandle);
	self->dbHandle = new ContactsDbHandle;

	TInt serverError = self->dbHandle->fileServer.Connect();
	if (serverError != KErrNone) {
		SAFE_CLEAR_REFERENCE(self->dbHandle);
		RETURN_SYMBIAN_ERROR(serverError);
	}
	
	
	CPbkContactEngine* contactEngine = NULL;
	TRAPD(error, { 
		if (!filename) {
			// Open default db file.
		contactEngine = CPbkContactEngine::NewL(&self->dbHandle->fileServer);
		} else {
			TBuf<KMaxFileName> filenameDes;
			error = convert_c_path_to_unicode_realpath(filenameDes, filename);
			User::LeaveIfError(error);
			switch (flag) {
			case 'e':
				// Open given db file, raise exception if the file does not exist.
				if (!doesFileExistL(filenameDes)) {
					User::Leave(KErrNotFound);
				}
				// * FALLTHROUGH *
			case 'c':	
				// Open, create if file doesn't exist.
				contactEngine = CPbkContactEngine::NewL(filenameDes, EFalse, &self->dbHandle->fileServer);
				break;
			case 'n':
				// Create a new empty file.
				contactEngine = CPbkContactEngine::NewL(filenameDes, ETrue, &self->dbHandle->fileServer);
				break;
			}
		}
	});
	if (error != KErrNone || !contactEngine) {
		SAFE_CLEAR_REFERENCE(self->dbHandle);
		RETURN_SYMBIAN_ERROR(error);
	}
	
	self->dbHandle->contactEngine = contactEngine;
	RETURN_TRUE;
}


/**
 * <bool> s60_contacts_Database::open([<string> filename [,<string> flag='e']])
 * open() - opens the default contact database file
 * open($filename [, 'e']) - opens file if it exists. Returns false if it doesn't.
 * open($filename , 'c') - opens file, creates new database if the file does not exist.
 * open($filename , 'n') - creates new empty database file and opens it.
 */
PHP_METHOD(s60_contacts_Database, open)
{
	char* filename = NULL;
	int filename_len;
	char* flag_str = NULL;
	int flag_str_len;
	
	GET_METHOD_ARGS_4(s60_contacts_Database, "O|ss", &filename, &filename_len, &flag_str, &flag_str_len);
	s60_contacts_do_open(self, filename, filename_len, flag_str, flag_str_len, return_value);
}

/**
 * <s60_contacts_Database> s60_contacts_open([<string> filename [,<string> flag='e']])
 * Returns an open <s60_contacts_Database> object. (or NULL on error)
 * s60_contacts_open() - opens the default contact database file
 * s60_contacts_open($filename [, 'e']) - opens file if it exists. Returns false if it doesn't.
 * s60_contacts_open($filename , 'c') - opens file, creates new database if the file does not exist.
 * s60_contacts_open($filename , 'n') - creates new empty database file and opens it.
 */
PHP_FUNCTION(s60_contacts_open)
{
	char* filename = NULL;
	int filename_len;
	char* flag_str = NULL;
	int flag_str_len;
	
	GET_FUNCTION_ARGS_4("|ss", &filename, &filename_len, &flag_str, &flag_str_len);
	zval new_object;
	INIT_ZVAL(new_object);
	if (object_init_ex(&new_object, CLASS_ENTRY(s60_contacts_Database)) == SUCCESS) {
		s60_contacts_do_open(GET_STRUCT(&new_object, s60_contacts_Database),
		                     filename, filename_len, flag_str, flag_str_len, return_value);
		if (IS_RETVAL_TRUE()) {
			*return_value = new_object;
		} else {
			zval_dtor(&new_object);	
		}
	} else {
		RETURN_ERROR("Could not allocate Database");
	}
}


/**
 * <bool> s60_contacts_Database::close()
 */
PHP_METHOD(s60_contacts_Database, close)
{
	GET_METHOD_ARGS_0(s60_contacts_Database);
	SAFE_CLEAR_REFERENCE(self->dbHandle);
	RETURN_TRUE;
}


#define ASSERT_DB_OPEN { if (!self->dbHandle) { RETURN_ERROR("Database not open"); } }

/**********************************************************************
 * Class s60_contacts_Contact
 * This class represents a single contact in a database.
 * A contact can have several fields such as first name, mobile
 * phone number and so on. See <s60_contacts_Field>.
 **********************************************************************/
 
#define CONTACT_NOT_OPEN 0x0
#define CONTACT_READ_ONLY 0x1
#define CONTACT_READ_WRITE 0x2

class ContactHandle : public Refcounted
{
	~ContactHandle()
	{
		delete_entry();
		SAFE_CLEAR_REFERENCE(dbHandle);
	}
	
public:
	ContactHandle() : mode(CONTACT_NOT_OPEN), uniqueId(KNullContactId), contactItem(0), dbHandle(0) {}

	void delete_entry()
	{
		delete contactItem;
		contactItem = 0;
		mode = CONTACT_NOT_OPEN;
	}

	TInt mode;
	TContactItemId uniqueId;
	CPbkContactItem* contactItem;
	ContactsDbHandle* dbHandle;	
};


BEGIN_CLASS_STRUCT(s60_contacts_Contact)
	ContactHandle* cHandle;
END_CLASS_STRUCT()

DEFINE_CLASS_DATA(s60_contacts_Contact)

BEGIN_CLASS_FUNCTIONS(s60_contacts_Contact)
	PHP_ME(s60_contacts_Contact, __construct, NULL, ZEND_ACC_PRIVATE)
	PHP_ME(s60_contacts_Contact, id, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Contact, title, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Contact, last_modified, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Contact, remove, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Contact, begin, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Contact, commit, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Contact, rollback, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Contact, fields, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Contact, find_fields, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Contact, add_field, NULL, ZEND_ACC_PUBLIC)
END_CLASS_FUNCTIONS()

DEFINE_OBJECT_FREE_FUNCTION(s60_contacts_Contact, {
	SAFE_CLEAR_REFERENCE(self->cHandle);
})
DEFINE_OBJECT_NEW_FUNCTION(s60_contacts_Contact, {
	self->cHandle = new ContactHandle;
})
DEFINE_CLASS_REGISTER_FUNCTION(s60_contacts_Contact)


PHP_METHOD(s60_contacts_Contact, __construct)
{
	RETURN_ERROR("Do not create Contact objects manually (use Database::get_contact)");
}

/*
 * Initialize given ContactHandle object. This is the "true" constructor.
 * if (uniqueId == KNullContactId) completely new contact entry is created.
 * Else only new contact object is created (wrapper object to 
 * existing contact entry that the uniqueId identifies).
 */
static TInt initialize_contact_object(ContactsDbHandle* dbHandle, ContactHandle* contact, TContactItemId uniqueId, bool is_read_only)
{
	if (!dbHandle) { return KErrGeneral; }
  
	if (uniqueId == KNullContactId){        
		// a new contact entry must be created into the database.
		TRAPD(error, {
			CPbkContactItem* newContact = 
				dbHandle->contactEngine->CreateEmptyContactL();

			// Remove the default fields so the contact really is empty
			while (newContact->CardFields().Count()) {
				newContact->RemoveField(newContact->CardFields().Count()-1);
			}

			contact->contactItem = newContact;
			contact->mode = CONTACT_READ_WRITE;
		});
		
		if (error != KErrNone){
			return error;
		} 
	} else {
		// contact entry that corresponds to given unique id exists.
		TRAPD(error, {
		if (is_read_only) {
			contact->contactItem = dbHandle->contactEngine->ReadContactL(uniqueId); 
			contact->mode = CONTACT_READ_ONLY;
		} else {
			contact->contactItem = dbHandle->contactEngine->OpenContactL(uniqueId);
			contact->mode = CONTACT_READ_WRITE;
		}
		});
		
		if (error != KErrNone) {
			return error;
		} 
	}
	contact->dbHandle = dbHandle;
	contact->dbHandle->incRef();
	contact->uniqueId = uniqueId;
	return KErrNone;
}

static TInt create_contact_object(ContactsDbHandle* dbHandle, zval* object, TContactItemId uniqueId)
{
	TSRMLS_FETCH(); 
	
	if (object_init_ex(object, CLASS_ENTRY(s60_contacts_Contact)) == SUCCESS) {
		return initialize_contact_object(
			dbHandle,
			GET_STRUCT(object, s60_contacts_Contact)->cHandle,
			uniqueId,
			true);
	}
	return KErrGeneral;
}

/**
 * This closes self->cHandle and creates a new object that has the requested
 * mode (is_read_only or not). The new object has same dbHandle and uniqueId.
 */
static TInt reopen_contact_object(CLASS_STRUCT(s60_contacts_Contact)* self, bool is_read_only)
{
	ContactsDbHandle* dbHandle = self->cHandle->dbHandle;
	TContactItemId uniqueId = self->cHandle->uniqueId;
	ContactHandle* oldHandle = self->cHandle;
	self->cHandle = new ContactHandle;
	// First create new object (increases dbHandle ref)
	TInt retval = initialize_contact_object(dbHandle, self->cHandle, uniqueId, is_read_only);
	// And only after that clear the old object (may decrease dbHandle--if this
	// happened before incRef, dbHandle object might die)
	SAFE_CLEAR_REFERENCE(oldHandle);
	return retval;
}


/**
 * <s60_contacts_Contact> s60_contacts_Database::add_contact()
 * Returns a new empty <s60_contacts_Contact> object.
 */
PHP_METHOD(s60_contacts_Database, add_contact)
{
	GET_METHOD_ARGS_0(s60_contacts_Database);
	ASSERT_DB_OPEN;
	TInt error = create_contact_object(self->dbHandle, return_value, KNullContactId);
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <s60_contacts_Contact> s60_contacts_Database::get_contact(<int> id)
 * Returns a <s60_contacts_Contact> object corresponding to given ID.
 */
PHP_METHOD(s60_contacts_Database, get_contact)
{
	long uniqueId;
	GET_METHOD_ARGS_1(s60_contacts_Database, "Ol", &uniqueId);
	ASSERT_DB_OPEN;
	
	if (uniqueId == KNullContactId) {
		RETURN_ERROR("Bad contact ID");	
	}
	
	TInt error = create_contact_object(self->dbHandle, return_value, uniqueId);
	RETURN_IF_SYMBIAN_ERROR(error);
}

/*
static void doSearchL(char *searchString, int searchString_len, zval* array, ContactsDbHandle* dbHandle)
{
	array_init(array);
	HBufC* searchStringBuf = convert_c_string_to_unicodeLC(searchString, searchString_len);
	CContactIdArray* idArray = dbHandle->contactEngine->FindLC(*searchStringBuf);
	
	for (TInt i=0; i < idArray->Count(); i++) {
		TContactItemId uniqueId = (*idArray)[i];
		// add contact with uniqueId to list
		zval *new_object;
		ALLOC_INIT_ZVAL(new_object);
		// create php object
		TInt error = create_contact_object(dbHandle, new_object, uniqueId);
		if (error != KErrNone) {
			zval_ptr_dtor(&new_object);		
			User::Leave(error);	
		}
		// append object to list
		add_next_index_zval(array, new_object);
	}
	CleanupStack::PopAndDestroy(idArray);
	CleanupStack::PopAndDestroy(searchStringBuf);
}
*/

/**
 * <array of s60_contacts_Contact> s60_contacts_Database::find(<string> query)
 * Searches contacts by string (match is detected if the string is
 * a substring of some field value of the contact).
PHP_METHOD(s60_contacts_Database, find)
{
	char *searchString;
	int searchString_len;
	GET_METHOD_ARGS_2(s60_contacts_Database, "Os", &searchString, &searchString_len);
	ASSERT_DB_OPEN;

	TRAPD(error, doSearchL(searchString, searchString_len, return_value, self->dbHandle));
	RETURN_IF_SYMBIAN_ERROR(error);
}
 */

/**
 * <array of s60_contacts_Contact> s60_contacts_Database::contacts()
 * Returns all contacts.
 */
PHP_METHOD(s60_contacts_Database, contacts)
{
	GET_METHOD_ARGS_0(s60_contacts_Database);
	ASSERT_DB_OPEN;

	array_init(return_value);
	TRAPD(error, {
		CPbkContactIter* iter = self->dbHandle->contactEngine->CreateContactIteratorLC();
		TInt uniqueId = iter->FirstL();
		while (uniqueId != KNullContactId) {
			// add contact with uniqueId to list
			zval *new_object;
			ALLOC_INIT_ZVAL(new_object);
			// create php object
			TInt error = create_contact_object(self->dbHandle, new_object, uniqueId);
			if (error != KErrNone) {
				zval_ptr_dtor(&new_object);			
				User::Leave(error);	
			}
			// append object to list
			add_next_index_zval(return_value, new_object);
			// get next unique id
			uniqueId = iter->NextL();
		}
		CleanupStack::PopAndDestroy(iter);
	});
	
	RETURN_IF_SYMBIAN_ERROR(error);
}

// in theory contactEngine should never be null
#define ASSERT_CONTACT_OPEN \
  if (self->cHandle->mode == CONTACT_NOT_OPEN || self->cHandle->dbHandle == NULL || self->cHandle->dbHandle->contactEngine == NULL) { \
    RETURN_ERROR("Contact not open"); \
  }
  
#define ASSERT_CONTACT_OPEN_RW \
  if (self->cHandle->mode != CONTACT_READ_WRITE || self->cHandle->dbHandle == NULL || self->cHandle->dbHandle->contactEngine == NULL) { \
    RETURN_ERROR("Contact not open for writing"); \
  }

/**
 * <int> s60_contacts_Contact::id()
 * Unique identifier for contact. To be used with <s60_contacts_Database>::get_contact()
 */
PHP_METHOD(s60_contacts_Contact, id)
{
	GET_METHOD_ARGS_0(s60_contacts_Contact);
	ASSERT_CONTACT_OPEN;
	RETURN_LONG(self->cHandle->uniqueId);
}

/**
 * <string> s60_contacts_Contact::title()
 */
PHP_METHOD(s60_contacts_Contact, title)
{
	GET_METHOD_ARGS_0(s60_contacts_Contact);
	ASSERT_CONTACT_OPEN;
	
	HBufC* titleBuf = NULL;
	TRAPD(error, titleBuf = self->cHandle->contactItem->GetContactTitleL());
	RETURN_IF_SYMBIAN_ERROR(error);
	
	error = ZVAL_SYMBIAN_STRING(return_value, *titleBuf);
	delete titleBuf;
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <float> s60_contacts_Contact::last_modified()
 */
PHP_METHOD(s60_contacts_Contact, last_modified)
{
	GET_METHOD_ARGS_0(s60_contacts_Contact);
	ASSERT_CONTACT_OPEN;
	RETURN_DOUBLE(convert_time_to_utc_real(self->cHandle->contactItem->ContactItem().LastModified()));
}

/**
 * s60_contacts_Contact::remove()
 * Removes the contact. After calling this, the object is useless
 * as it no longer refers to anything.
 */
PHP_METHOD(s60_contacts_Contact, remove)
{
	GET_METHOD_ARGS_0(s60_contacts_Contact);
	ASSERT_CONTACT_OPEN;
	TRAPD(error, self->cHandle->dbHandle->contactEngine->DeleteContactL(self->cHandle->uniqueId));
	if (error == KErrInUse) {
		RETURN_ERROR("Contact is open for writing and cannot be deleted (call commit first)");	
	}
	RETURN_IF_SYMBIAN_ERROR(error);
	self->cHandle->delete_entry();
	RETURN_TRUE;
}

/**
 * s60_contacts_Contact::begin()
 * Opens contact for writing. Must be called for contacts received from
 * e.g. <s60_contacts_Database>::get_contact(), before editing.
 */
PHP_METHOD(s60_contacts_Contact, begin)
{
	GET_METHOD_ARGS_0(s60_contacts_Contact);
	ASSERT_CONTACT_OPEN;
	if (self->cHandle->mode == CONTACT_READ_WRITE) {
		RETURN_TRUE;
	}
	if (self->cHandle->uniqueId == KNullContactId) {
		RETURN_ERROR("New contacts are already writable");
	}
	TInt error = reopen_contact_object(self, false);
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}

/**
 * s60_contacts_Contact::commit()
 * Commits the contact. Unless this is called, no changes are made.
 */
PHP_METHOD(s60_contacts_Contact, commit)
{
	GET_METHOD_ARGS_0(s60_contacts_Contact);
	ASSERT_CONTACT_OPEN_RW;	
	TRAPD(error, {
		if (self->cHandle->uniqueId == KNullContactId) {
			// New contact.
			self->cHandle->uniqueId = 
				self->cHandle->dbHandle->contactEngine->AddNewContactL(*self->cHandle->contactItem);
		} else {
			// Old contact (begin() called).
			self->cHandle->dbHandle->contactEngine->CommitContactL(*self->cHandle->contactItem);
		}
	});
	RETURN_IF_SYMBIAN_ERROR(error);
	error = reopen_contact_object(self, true);
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}

/**
 * s60_contacts_Contact::rollback()
 * All changes to contact since last call to begin() will disappear.
 */
PHP_METHOD(s60_contacts_Contact, rollback)
{
	GET_METHOD_ARGS_0(s60_contacts_Contact);
	ASSERT_CONTACT_OPEN_RW;
	if (self->cHandle->uniqueId == KNullContactId) {
		RETURN_ERROR("New contacts can't be rolled back");
	}
	TRAPD(error, self->cHandle->dbHandle->contactEngine->CloseContactL(self->cHandle->uniqueId));
	RETURN_IF_SYMBIAN_ERROR(error);
	error = reopen_contact_object(self, true);
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}


/**********************************************************************
 * Class s60_contacts_FieldType
 * Represents a type of a contact field. A contact item contains
 * several fields (see <s60_contacts_Field>), each of which has
 * some type.
 **********************************************************************/
 
BEGIN_CLASS_STRUCT(s60_contacts_FieldType)
	ContactsDbHandle* dbHandle;
	ContactHandle* cHandle;
	const CPbkFieldInfo* fieldInfo;
END_CLASS_STRUCT()

DEFINE_CLASS_DATA(s60_contacts_FieldType)

BEGIN_CLASS_FUNCTIONS(s60_contacts_FieldType)
	PHP_ME(s60_contacts_FieldType, __construct, NULL, ZEND_ACC_PRIVATE)
	PHP_ME(s60_contacts_FieldType, name, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, id, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, location, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, storage_type, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, multi, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, is_read_only, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, is_name, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, is_numeric, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, is_phone_number, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, is_email, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, is_email_over_sms, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, is_mms, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, is_image, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, is_editable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, is_addable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_FieldType, get_add_label, NULL, ZEND_ACC_PUBLIC)
END_CLASS_FUNCTIONS()

DEFINE_OBJECT_FREE_FUNCTION(s60_contacts_FieldType, {
	SAFE_CLEAR_REFERENCE(self->dbHandle);
	SAFE_CLEAR_REFERENCE(self->cHandle);
})
DEFINE_OBJECT_NEW_FUNCTION(s60_contacts_FieldType, {
	self->dbHandle = 0;
	self->cHandle = 0;
})
DEFINE_CLASS_REGISTER_FUNCTION(s60_contacts_FieldType)

/**
 * <array of s60_contacts_FieldType> s60_contacts_Database::field_types()
 * Returns all field types.
 */
PHP_METHOD(s60_contacts_Database, field_types)
{
	GET_METHOD_ARGS_0(s60_contacts_Database);
	ASSERT_DB_OPEN
	const CPbkFieldsInfo& fieldsInfo = self->dbHandle->contactEngine->FieldsInfo();
	array_init(return_value);
	for (TInt i=0; i < fieldsInfo.Count();i++) { 
		zval *new_object;
		ALLOC_INIT_ZVAL(new_object);
		if (object_init_ex(new_object, CLASS_ENTRY(s60_contacts_FieldType)) == SUCCESS) {
			GET_STRUCT(new_object, s60_contacts_FieldType)->fieldInfo = fieldsInfo[i];
			GET_STRUCT(new_object, s60_contacts_FieldType)->dbHandle = self->dbHandle;
			self->dbHandle->incRef();
			add_next_index_zval(return_value, new_object);
		} else {
			zval_ptr_dtor(&new_object);
			RETURN_ERROR("Could not allocate all field infos");
		}
	}
}

PHP_METHOD(s60_contacts_FieldType, __construct)
{
	RETURN_ERROR("Do not create FieldType objects manually (use Database::field_types)");
}

// again, this should never fail
#define ASSERT_FIELD_INFO { if (!self->dbHandle && !self->cHandle) { RETURN_ERROR("Database not open"); } }

/**
 * <string> s60_contacts_FieldType::name()
 * Returns field name
 */
PHP_METHOD(s60_contacts_FieldType, name)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	TInt error = ZVAL_SYMBIAN_STRING(return_value, self->fieldInfo->FieldName());
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * <int> s60_contacts_FieldType::id()
 * Returns field id (e.g. S60_CONTACTS_LAST_NAME)
 */
PHP_METHOD(s60_contacts_FieldType, id)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_LONG(self->fieldInfo->FieldId());
}

/**
 * <int> s60_contacts_FieldType::location()
 * Returns field location (e.g. S60_CONTACTS_LOCATION_WORK)
 */
PHP_METHOD(s60_contacts_FieldType, location)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_LONG(self->fieldInfo->Location());
}

/**
 * <int> s60_contacts_FieldType::storage_type()
 * Returns field storage type (e.g. S60_CONTACTS_STORAGE_TEXT)
 */
PHP_METHOD(s60_contacts_FieldType, storage_type)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_LONG(self->fieldInfo->FieldStorageType());
}

/**
 * <int> s60_contacts_FieldType::multi()
 * Returns field multiplicity (e.g. S60_CONTACTS_MULTI_ONE)
 * S60_CONTACTS_MULTI_ONE means that there may be zero or one
 *  fields of that type.
 * S60_CONTACTS_MULTI_MANY means that there may be zero or more
 *  fields of that type.
 */
PHP_METHOD(s60_contacts_FieldType, multi)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_LONG(self->fieldInfo->Multiplicity());
}
//XXX      "maxlength", fieldsInfo[index]->MaxLength(),           

/**
 * <bool> s60_contacts_FieldType::is_read_only()
 */
PHP_METHOD(s60_contacts_FieldType, is_read_only)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_BOOL(self->fieldInfo->IsReadOnly());
}

/**
 * <bool> s60_contacts_FieldType::is_name()
 * S60_CONTACTS_FIRST_NAME and S60_CONTACTS_LAST_NAME are names.
 */
PHP_METHOD(s60_contacts_FieldType, is_name)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_BOOL(self->fieldInfo->NameField());
}

/**
 * <bool> s60_contacts_FieldType::is_numeric()
 * Numeric types include phone numbers and e.g. P.O. Boxes.
 */
PHP_METHOD(s60_contacts_FieldType, is_numeric)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_BOOL(self->fieldInfo->NumericField());
}

/**
 * <bool> s60_contacts_FieldType::is_phone_number()
 */
PHP_METHOD(s60_contacts_FieldType, is_phone_number)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_BOOL(self->fieldInfo->IsPhoneNumberField());
}

/**
 * <bool> s60_contacts_FieldType::is_email()
 */
PHP_METHOD(s60_contacts_FieldType, is_email)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_BOOL(self->fieldInfo->IsEmailField());
}

/**
 * <bool> s60_contacts_FieldType::is_email_over_sms()
 * This is similar (or same) as is_email() || is_phone_number()
 */
PHP_METHOD(s60_contacts_FieldType, is_email_over_sms)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_BOOL(self->fieldInfo->IsEmailOverSmsField());
}

/**
 * <bool> s60_contacts_FieldType::is_mms()
 */
PHP_METHOD(s60_contacts_FieldType, is_mms)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_BOOL(self->fieldInfo->IsMmsField());
}

/**
 * <bool> s60_contacts_FieldType::is_image()
 */
PHP_METHOD(s60_contacts_FieldType, is_image)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_BOOL(self->fieldInfo->IsImageField());
}

/**
 * <bool> s60_contacts_FieldType::is_editable()
 * Is the field type editable?
 */
PHP_METHOD(s60_contacts_FieldType, is_editable)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_BOOL(self->fieldInfo->IsEditable());
}

/**
 * <bool> s60_contacts_FieldType::is_addable()
 * Can user add this type of field?
 */
PHP_METHOD(s60_contacts_FieldType, is_addable)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	RETURN_BOOL(self->fieldInfo->UserCanAddField());
}


/**
 * <string> s60_contacts_FieldType::get_add_label()
 * Returns a string representing the field type label
 * shown when field type is added. Usually this is
 * identical to name().
 */
PHP_METHOD(s60_contacts_FieldType, get_add_label)
{
	GET_METHOD_ARGS_0(s60_contacts_FieldType);
	ASSERT_FIELD_INFO;
	TInt error = ZVAL_SYMBIAN_STRING(return_value, self->fieldInfo->AddItemText());
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**********************************************************************
 * Class s60_contacts_Field
 * This class represents a single field within some contact.
 **********************************************************************/
 
BEGIN_CLASS_STRUCT(s60_contacts_Field)
	TPbkContactItemField* field;
	ContactHandle* cHandle;
END_CLASS_STRUCT()

DEFINE_CLASS_DATA(s60_contacts_Field)

BEGIN_CLASS_FUNCTIONS(s60_contacts_Field)
	PHP_ME(s60_contacts_Field, __construct, NULL, ZEND_ACC_PRIVATE)
	PHP_ME(s60_contacts_Field, type, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Field, label, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Field, set_label, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Field, value, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Field, set_value, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(s60_contacts_Field, remove, NULL, ZEND_ACC_PUBLIC)
END_CLASS_FUNCTIONS()

DEFINE_OBJECT_FREE_FUNCTION(s60_contacts_Field, {
	delete self->field;
	SAFE_CLEAR_REFERENCE(self->cHandle);
})
DEFINE_OBJECT_NEW_FUNCTION(s60_contacts_Field, {
	self->cHandle = 0;
	self->field = 0;
})
DEFINE_CLASS_REGISTER_FUNCTION(s60_contacts_Field)

#define ASSERT_FIELD { if (!self->field || !self->cHandle || self->cHandle->mode == CONTACT_NOT_OPEN) { RETURN_ERROR("Field not open"); } }
#define ASSERT_FIELD_CONTACT_READ_WRITE { if (!self->field || !self->cHandle || self->cHandle->mode != CONTACT_READ_WRITE) { RETURN_ERROR("Contact not open for writing"); } }


PHP_METHOD(s60_contacts_Field, __construct)
{
	RETURN_ERROR("Do not create Field objects manually (use Contact::fields)");
}

/**
 * <s60_contacts_FieldType> s60_contacts_Field::type()
 */
PHP_METHOD(s60_contacts_Field, type)
{
	GET_METHOD_ARGS_0(s60_contacts_Field);
	ASSERT_FIELD;
	
	if (object_init_ex(return_value, CLASS_ENTRY(s60_contacts_FieldType)) == SUCCESS) {
		//XXX contactItem owns CardFields which owns fields. Hopefully FieldInfo
		//is owned there as well. At least no fieldInfo is ever explicitely released..
		//So FieldType will have a reference to the contact which holds the fieldInfo alive.
		GET_STRUCT(return_value, s60_contacts_FieldType)->fieldInfo = &self->field->FieldInfo();
		GET_STRUCT(return_value, s60_contacts_FieldType)->cHandle = self->cHandle;
		self->cHandle->incRef();
	} else {
		RETURN_ERROR("Could not allocate field info");
	}
}

/**
 * <string> s60_contacts_Field::label()
 */
PHP_METHOD(s60_contacts_Field, label)
{
	GET_METHOD_ARGS_0(s60_contacts_Field);
	ASSERT_FIELD;
	TInt error = ZVAL_SYMBIAN_STRING(return_value, self->field->Label());
	RETURN_IF_SYMBIAN_ERROR(error);
}

/**
 * s60_contacts_Field::set_label(<string> new_label)
 */
PHP_METHOD(s60_contacts_Field, set_label)
{
	char* label;
	int label_len;
	GET_METHOD_ARGS_2(s60_contacts_Field, "Os", &label, &label_len);
	ASSERT_FIELD_CONTACT_READ_WRITE;
	TRAPD(error, {
		HBufC* buf = convert_c_string_to_unicodeLC(label, label_len);
		self->field->SetLabelL(*buf);
		CleanupStack::PopAndDestroy(buf);
	});
	RETURN_IF_SYMBIAN_ERROR(error);
	RETURN_TRUE;
}

/**
 * <string or float> s60_contacts_Field::value()
 * Returns string for text fields (also phone numbers), and
 * float for datetime fields.
 */
PHP_METHOD(s60_contacts_Field, value)
{
	GET_METHOD_ARGS_0(s60_contacts_Field);
	ASSERT_FIELD;
	
	switch (self->field->FieldInfo().FieldStorageType()) {
	case KStorageTypeText:
	{
		TInt error = ZVAL_SYMBIAN_STRING(return_value, self->field->Text());
		RETURN_IF_SYMBIAN_ERROR(error);
		return;
	}
	case KStorageTypeDateTime:
	{
		double retval = convert_time_to_utc_real(self->field->Time());
		RETURN_DOUBLE(retval);
	}
	}
	RETURN_ERROR("Field type not supported");
}


/**
 * s60_contacts_Field::set_value(<string or float> new_value)
 * Assigns string for text fields (also phone numbers), and
 * float for datetime fields.
 */
PHP_METHOD(s60_contacts_Field, set_value)
{
	zval* value;
	GET_METHOD_ARGS_1(s60_contacts_Field, "Oz", &value);
	ASSERT_FIELD_CONTACT_READ_WRITE;
	
	switch (self->field->StorageType()) {
	case KStorageTypeText:
	{
		convert_to_string(value);
		TRAPD(error, {
			HBufC* buf = convert_c_string_to_unicodeLC(Z_STRVAL_P(value), Z_STRLEN_P(value));
			self->field->TextStorage()->SetTextL(*buf);
			CleanupStack::PopAndDestroy(buf);
		});
		RETURN_IF_SYMBIAN_ERROR(error);
		RETURN_TRUE;
	}
		
	case KStorageTypeDateTime:
	{
		convert_to_double(value);
		TTime time = convert_utc_real_to_time(Z_DVAL_P(value));
		self->field->DateTimeStorage()->SetTime(time.DateTime());
		RETURN_TRUE;
	}
	}
	RETURN_ERROR("Field type not supported");
}


/**
 * s60_contacts_Field::remove()
 * Removes the field. After calling this, the object is useless 
 * as it no longer refers to anything.
 */
PHP_METHOD(s60_contacts_Field, remove)
{
	GET_METHOD_ARGS_0(s60_contacts_Field);
	ASSERT_FIELD_CONTACT_READ_WRITE;
	self->cHandle->contactItem->RemoveField(self->cHandle->contactItem->FindFieldIndex(*self->field));
	self->field = 0;
	RETURN_TRUE;
}


static TInt create_field_object(zval* object, TPbkContactItemField* field, ContactHandle* cHandle)
{
	TSRMLS_FETCH(); 
	
	if (object_init_ex(object, CLASS_ENTRY(s60_contacts_Field)) == SUCCESS) {
		GET_STRUCT(object, s60_contacts_Field)->field = new TPbkContactItemField(*field);
		GET_STRUCT(object, s60_contacts_Field)->cHandle = cHandle;
		cHandle->incRef();
		return KErrNone;
	}
	return KErrGeneral;
}


static TInt append_field(zval* array, TPbkContactItemField* field, ContactHandle* cHandle)
{
	// add field to list
	zval *new_object;
	ALLOC_INIT_ZVAL(new_object);
	// create php object
	TInt error = create_field_object(new_object, field, cHandle);
	if (error != KErrNone) {
		zval_ptr_dtor(&new_object);
		return error;
	}
	// append object to list
	add_next_index_zval(array, new_object);
	return KErrNone;
}


/**
 * <array of s60_contacts_Field> s60_contacts_Contact::fields()
 * Returns all fields for the contact.
 */
PHP_METHOD(s60_contacts_Contact, fields)
{
	GET_METHOD_ARGS_0(s60_contacts_Contact);
	ASSERT_CONTACT_OPEN;
	
	array_init(return_value);
	CPbkFieldArray& fieldArray = self->cHandle->contactItem->CardFields();
	for (TInt i = 0; i < fieldArray.Count(); ++i) {
		TInt error = append_field(return_value, &fieldArray[i], self->cHandle);
		RETURN_IF_SYMBIAN_ERROR(error);
	}
}

/**
 * <array of s60_contacts_Field> s60_contacts_Contact::find_fields(<int> fieldId [, <int> location])
 * Returns fields that match the search criteria. For a specific FieldType $ft
 * one can find those fields by calling find_fields($ft->id(), $ft->location())
 * Alternatively one could do e.g. find_fields(S60_CONTACTS_FIRST_NAME).
 */
PHP_METHOD(s60_contacts_Contact, find_fields)
{
	long fieldId;
	long location = -1;
	
	GET_METHOD_ARGS_2(s60_contacts_Contact, "Ol|l", &fieldId, &location);
	ASSERT_CONTACT_OPEN;

	array_init(return_value);
	TInt fieldIndex = 0;
	TPbkContactItemField* field;
	while ((field = self->cHandle->contactItem->FindField(fieldId, fieldIndex)) != 0) {
		if (location == -1 || field->FieldInfo().Location() == location){
			TInt error = append_field(return_value, field, self->cHandle);
			RETURN_IF_SYMBIAN_ERROR(error);
		}
		fieldIndex++;
	}
}

/**
 * <s60_contacts_Field> s60_contacts_Contact::add_field(<int> fieldId [, <int> location])
 * Adds a new field with default label and empty value,
 * and returns a field object.
 * Example:
 *  $f = $contact->add_field(S60_CONTACTS_EMAIL, S60_CONTACTS_LOCATION_WORK);
 *  $f->set_value("my_work@email.com");
 */
PHP_METHOD(s60_contacts_Contact, add_field)
{
	long fieldId;
	long location = -1;
	//XXX accepting FieldType as a parameter would be nice
	// now one must pass in FieldType::id() and FieldType::location()
	GET_METHOD_ARGS_2(s60_contacts_Contact, "Ol|l", &fieldId, &location);
	ASSERT_CONTACT_OPEN_RW;
  
	const CPbkFieldsInfo& fieldsInfo = self->cHandle->dbHandle->contactEngine->FieldsInfo();
	CPbkFieldInfo* fieldInfo;
	
	if (location == -1) {
		fieldInfo = fieldsInfo.Find(fieldId);	
	} else {
		fieldInfo = fieldsInfo.Find(fieldId, (TPbkFieldLocation)location);	
	}
	if (!fieldInfo) {
		RETURN_ERROR("No such field type");
	}
	
	TPbkContactItemField* field = NULL;
	TRAPD(error, field = &(self->cHandle->contactItem->AddFieldL(*fieldInfo)));
	RETURN_IF_SYMBIAN_ERROR(error);
	error = create_field_object(return_value, field, self->cHandle);
	RETURN_IF_SYMBIAN_ERROR(error);
}



} /* extern "C" */

