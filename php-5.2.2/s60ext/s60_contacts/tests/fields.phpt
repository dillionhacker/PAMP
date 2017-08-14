--TEST--
Testing s60_contacts fields and db::get_contact.
--FILE--
<?php

function test() {
	$db = s60_contacts_open("ctest.db", "n");
	
	// create new contact and print out its info
	echo "adding a new contact\n";
	$c = $db->add_contact();
	$f = $c->add_field(S60_CONTACTS_FIRST_NAME);
	$f->set_value("frst");
	$f->set_label("eka");
	$c->add_field(S60_CONTACTS_LAST_NAME)->set_value("ugo");
	$c->commit();
	$id1 = $c->id();
	
	echo "adding a new contact\n";
	$c = $db->add_contact();
	$c->add_field(S60_CONTACTS_FIRST_NAME)->set_value("joku");
	$f = $c->add_field(S60_CONTACTS_LAST_NAME);
	$f->set_value("ugo");
	$f->set_label("suku");
	$c->commit();
	$id2 = $c->id();
	
	unset($c);
	unset($db);
	$db = s60_contacts_open("ctest.db");
	
	$f1 = $db->get_contact($id1)->find_fields(S60_CONTACTS_FIRST_NAME);
	$f2 = $db->get_contact($id2)->find_fields(S60_CONTACTS_LAST_NAME);
	var_dump($f1[0]->label());
	var_dump($f1[0]->value());
	var_dump($f1[0]->type()->id() === S60_CONTACTS_FIRST_NAME);
	var_dump($f2[0]->label());
	var_dump($f2[0]->value());
	var_dump($f2[0]->type()->id() === S60_CONTACTS_LAST_NAME);
}

test();

 
?>
--EXPECTF--
adding a new contact
adding a new contact
string(3) "eka"
string(4) "frst"
bool(true)
string(4) "suku"
string(3) "ugo"
bool(true)
