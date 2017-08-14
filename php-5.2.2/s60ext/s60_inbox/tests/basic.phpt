--TEST--
Basic s60_inbox test.
--FILE--
<?php

var_dump(s60_inbox_send_message("==TESTMESSAGE==", "=TESTSENDER="));
$m = s60_inbox_get_messages();
if (count($m) < 1 || $m[0]->sender() !== "=TESTSENDER=") {
  die("Wrong sender!");
}
var_dump($m[0]->sender());
var_dump($m[0]->content());
var_dump($m[0]->time());

$m[0]->remove();
$m[0]->remove();
echo 'OK';

?>
--EXPECTF--
bool(true)
string(12) "=TESTSENDER="
string(15) "==TESTMESSAGE=="
float(%f)

Warning: Message not found in %s.php on line %d
OK