--TEST--
Try removing contacts at wrong places.
--FILE--
<?php

function test() {
	$db = s60_contacts_open("ctest.db", "n");
	$c = $db->add_contact();
	$c->remove();
	$c->commit();
	$c->begin();
	$c->remove();
	echo 'ok';
}


test();
 
 ?>
--EXPECTF--
Warning: Not found in %s.php on line 6

Warning: Contact is open for writing and cannot be deleted (call commit first) in %s.php on line 9
ok
