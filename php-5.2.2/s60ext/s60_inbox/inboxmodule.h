#ifndef PHP_S60_INBOX_H
#define PHP_S60_INBOX_H

PHP_FUNCTION(s60_inbox_send_message);
PHP_FUNCTION(s60_inbox_get_messages);

PHP_MINIT_FUNCTION(s60_inbox);

PHP_METHOD(s60_inbox_Message, __construct);
PHP_METHOD(s60_inbox_Message, content);
PHP_METHOD(s60_inbox_Message, time);
PHP_METHOD(s60_inbox_Message, flags);
PHP_METHOD(s60_inbox_Message, sender);
PHP_METHOD(s60_inbox_Message, remove);

extern zend_module_entry s60_inbox_module_entry;
#define phpext_s60_inbox_ptr &s60_inbox_module_entry

#endif

