--TEST--
Test adding all s60_contacts fields into one contact and modifying each label.
--FILE--
<?php

function test() {
	$db = s60_contacts_open("ctest.db", "n");
	
	$c = $db->add_contact();
	foreach ($db->field_types() as $ft) {
		if ($ft->storage_type() === S60_CONTACTS_STORAGE_TEXT && $ft->is_addable() && $ft->is_editable()) {
			$f = $c->add_field($ft->id(), $ft->location());
			$f->set_label("X");
			$f->set_value("Y");
		}
	}
	$c->commit();

	$c->begin();

	foreach ($db->field_types() as $ft) {
		if ($ft->storage_type() === S60_CONTACTS_STORAGE_TEXT && $ft->is_addable() && $ft->is_editable()) {
			$f = $c->find_fields($ft->id(), $ft->location());
			assert($f[0]->label() === "X");
			assert($f[0]->value() === "Y");
			$f[0]->remove();
			@$f[0]->remove();
		}
	}
	var_dump(count($c->fields()));
	echo 'ok';
}

test();
 
 ?>
--EXPECTF--
int(0)
ok
