#ifndef PHP_S60_CONTACTS_H
#define PHP_S60_CONTACTS_H

PHP_MINIT_FUNCTION(s60_contacts);

PHP_FUNCTION(s60_contacts_open);

PHP_METHOD(s60_contacts_Database, __construct);
PHP_METHOD(s60_contacts_Database, open);
PHP_METHOD(s60_contacts_Database, close);
PHP_METHOD(s60_contacts_Database, field_types);
PHP_METHOD(s60_contacts_Database, add_contact);
PHP_METHOD(s60_contacts_Database, get_contact);
//PHP_METHOD(s60_contacts_Database, find);
PHP_METHOD(s60_contacts_Database, contacts);

PHP_METHOD(s60_contacts_FieldType, __construct);
PHP_METHOD(s60_contacts_FieldType, name);
PHP_METHOD(s60_contacts_FieldType, id);
PHP_METHOD(s60_contacts_FieldType, location);
PHP_METHOD(s60_contacts_FieldType, storage_type);
PHP_METHOD(s60_contacts_FieldType, multi);
PHP_METHOD(s60_contacts_FieldType, is_read_only);
PHP_METHOD(s60_contacts_FieldType, is_name);
PHP_METHOD(s60_contacts_FieldType, is_numeric);
PHP_METHOD(s60_contacts_FieldType, is_phone_number);
PHP_METHOD(s60_contacts_FieldType, is_email);
PHP_METHOD(s60_contacts_FieldType, is_email_over_sms);
PHP_METHOD(s60_contacts_FieldType, is_mms);
PHP_METHOD(s60_contacts_FieldType, is_image);
PHP_METHOD(s60_contacts_FieldType, is_editable);
PHP_METHOD(s60_contacts_FieldType, is_addable);
PHP_METHOD(s60_contacts_FieldType, get_add_label);

PHP_METHOD(s60_contacts_Contact, __construct);
PHP_METHOD(s60_contacts_Contact, id);
PHP_METHOD(s60_contacts_Contact, title);
PHP_METHOD(s60_contacts_Contact, last_modified);
PHP_METHOD(s60_contacts_Contact, remove);
PHP_METHOD(s60_contacts_Contact, begin);
PHP_METHOD(s60_contacts_Contact, commit);
PHP_METHOD(s60_contacts_Contact, rollback);
PHP_METHOD(s60_contacts_Contact, fields);
PHP_METHOD(s60_contacts_Contact, find_fields);
PHP_METHOD(s60_contacts_Contact, add_field);

PHP_METHOD(s60_contacts_Field, __construct);
PHP_METHOD(s60_contacts_Field, type);
PHP_METHOD(s60_contacts_Field, label);
PHP_METHOD(s60_contacts_Field, value);
PHP_METHOD(s60_contacts_Field, set_label);
PHP_METHOD(s60_contacts_Field, set_value);
PHP_METHOD(s60_contacts_Field, remove);

extern zend_module_entry s60_contacts_module_entry;
#define phpext_s60_contacts_ptr &s60_contacts_module_entry

#endif

