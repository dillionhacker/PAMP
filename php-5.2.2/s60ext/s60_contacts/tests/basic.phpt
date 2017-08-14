--TEST--
Basic s60_contacts test.
--FILE--
<?php

function format_loc($loc) {
	switch ($loc) {
	case S60_CONTACTS_LOCATION_NONE:
		return "none";
	case S60_CONTACTS_LOCATION_HOME:
		return "home";
	case S60_CONTACTS_LOCATION_WORK:
		return "work";
	}
	return "error";
}

function format_multi($m) {
	switch ($m) {
	case S60_CONTACTS_MULTI_ONE:
		return "one";
	case S60_CONTACTS_MULTI_MANY:
		return "many";
	}
	return "error";
}



function print_field_type($ft) {
	printf("L:%-5s   M:%-5s | #:%3d   @:%3d\n", format_loc($ft->location()), format_multi($ft->multi()), $ft->is_phone_number(), $ft->is_email());
}

function print_field($f) {
	//echo "    LABEL: ".$f->label()."\n";
	echo "      VALUE: ".$f->value()."\n";
	echo "      TYPE: "; print_field_type($f->type());
}

function print_contact($c) {
	echo " ID: ".$c->id()."\n";
	echo " TITLE: ".$c->title()."\n";
	echo " MODIFIED: ".$c->last_modified()."\n";
	echo " FIELDS:\n";
	foreach ($c->fields() as $f) {
		print_field($f);
	}
}

function test() {
	$db = s60_contacts_open("ctest.db", "n");
	var_dump($db->get_contact(1));
	var_dump($db->get_contact(-111));
	
	// create new contact and print out its info
	echo "adding a new contact\n";
	$c = $db->add_contact();
	var_dump(count($c->fields()));
	$c->add_field(S60_CONTACTS_FIRST_NAME)->set_value("frst");
	$c->add_field(S60_CONTACTS_LAST_NAME)->set_value("lst");
	$c->add_field(S60_CONTACTS_DATE)->set_value(12345);
	$c->add_field(S60_CONTACTS_EMAIL)->set_value("ukko@XXX.fi");
	$c->add_field(S60_CONTACTS_EMAIL, S60_CONTACTS_LOCATION_HOME)->set_value("ukko@koti.fi");
	$c->add_field(S60_CONTACTS_EMAIL, S60_CONTACTS_LOCATION_WORK)->set_value("ukko@nokia.fi");
	$c->commit();
	var_dump(count($c->fields()));
	$c->begin();
	print_contact($c);
	foreach ($c->fields() as $f) {
		$f->remove();
	}
	var_dump(count($c->fields()));
	$c->rollback();
	var_dump(count($c->fields()));
	echo "adding a new contact\n";
      $c = $db->add_contact();
	var_dump(count($db->contacts()));
      $c->commit();
	var_dump(count($db->contacts()));

	// print out all your contacts
	echo "\n\nALL CONTACTS:\n";
	echo "=========================================\n";
	foreach ($db->contacts() as $c) {
		print_contact($c);
		$c->begin();
		$c->add_field(S60_CONTACTS_FAX, S60_CONTACTS_LOCATION_WORK)->set_value("+123456");
		$c->commit();
		print_contact($c);
		$c->remove();
		echo "--------------------------------\n";
	}
	var_dump(count($db->contacts()));
	
	// print out all available field types
	echo "\n\nFIELD TYPES:\n";
	echo "=========================================\n";
	$types = $db->field_types();
	var_dump($db->close());
	foreach ($types as $ft) {
		if ($ft->id() == S60_CONTACTS_FIRST_NAME || $ft->id() == S60_CONTACTS_EMAIL)
			print_field_type($ft);
	}
}

test();


?>
--EXPECTF--
Warning: Not found in %s.php on line 49
NULL

Warning: Not found in %s.php on line 50
NULL
adding a new contact
int(0)
int(6)
 ID: 1
 TITLE: lst frst
 MODIFIED: %f
 FIELDS:
      VALUE: frst
      TYPE: L:none    M:one   | #:  0   @:  0
      VALUE: lst
      TYPE: L:none    M:one   | #:  0   @:  0
      VALUE: ukko@XXX.fi
      TYPE: L:none    M:many  | #:  0   @:  1
      VALUE: ukko@koti.fi
      TYPE: L:home    M:many  | #:  0   @:  1
      VALUE: ukko@nokia.fi
      TYPE: L:work    M:many  | #:  0   @:  1
      VALUE: 12345
      TYPE: L:none    M:one   | #:  0   @:  0
int(0)
int(6)
adding a new contact
int(1)
int(2)


ALL CONTACTS:
=========================================
 ID: 1
 TITLE: lst frst
 MODIFIED: %f
 FIELDS:
      VALUE: frst
      TYPE: L:none    M:one   | #:  0   @:  0
      VALUE: lst
      TYPE: L:none    M:one   | #:  0   @:  0
      VALUE: ukko@XXX.fi
      TYPE: L:none    M:many  | #:  0   @:  1
      VALUE: ukko@koti.fi
      TYPE: L:home    M:many  | #:  0   @:  1
      VALUE: ukko@nokia.fi
      TYPE: L:work    M:many  | #:  0   @:  1
      VALUE: 12345
      TYPE: L:none    M:one   | #:  0   @:  0
 ID: 1
 TITLE: lst frst
 MODIFIED: %f
 FIELDS:
      VALUE: frst
      TYPE: L:none    M:one   | #:  0   @:  0
      VALUE: lst
      TYPE: L:none    M:one   | #:  0   @:  0
      VALUE: +123456
      TYPE: L:work    M:many  | #:  1   @:  0
      VALUE: ukko@XXX.fi
      TYPE: L:none    M:many  | #:  0   @:  1
      VALUE: ukko@koti.fi
      TYPE: L:home    M:many  | #:  0   @:  1
      VALUE: ukko@nokia.fi
      TYPE: L:work    M:many  | #:  0   @:  1
      VALUE: 12345
      TYPE: L:none    M:one   | #:  0   @:  0
--------------------------------
 ID: 2
 TITLE: (unnamed)
 MODIFIED: %f
 FIELDS:
 ID: 2
 TITLE: (unnamed)
 MODIFIED: %f
 FIELDS:
      VALUE: +123456
      TYPE: L:work    M:many  | #:  1   @:  0
--------------------------------
int(0)


FIELD TYPES:
=========================================
bool(true)
L:none    M:one   | #:  0   @:  0
L:none    M:many  | #:  0   @:  1
L:home    M:many  | #:  0   @:  1
L:work    M:many  | #:  0   @:  1