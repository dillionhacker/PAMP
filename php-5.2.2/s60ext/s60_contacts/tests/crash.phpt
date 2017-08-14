--TEST--
This s60_contacts may crash (but should not!)
--FILE--
<?php

function test() {
	$db = s60_contacts_open("ctest.db", "n");
	
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

	$fs = $c->fields();
	$c->begin();
	var_dump($fs[0]->value()); // crash? $c has closed and recreated the relevant object
	var_dump($fs[1]->value()); // crash? $c has closed and recreated the relevant object
}

test();


?>
--EXPECTF--
adding a new contact
int(0)
string(4) "frst"
string(3) "lst"
