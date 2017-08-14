--TEST--
Tests s60_calendar's EKA2-only features (REMINDER type, entry status)
--FILE--
<?php

function my_assert_handler($file, $line, $code)
{
	$lines = file($file);
	echo "\nFailure:".trim($lines[$line-1])."\n";
}
assert_options(ASSERT_CALLBACK, 'my_assert_handler');

$db = s60_calendar_open("ctest.db", "n");
$c = $db->add_entry(S60_CALENDAR_TYPE_REMINDER);
$c->set_content("cont");
$c->set_location("loca");
$c->set_start_end_time(1155500000,1155600000);
$c->set_status(S60_CALENDAR_STATUS_TODO_COMPLETED); //fail
echo "again..\n";
$c->set_status(S60_CALENDAR_STATUS_TENTATIVE);
$c->commit();

$i1 = $db->find_instances_day(1155500000);
$e = $i1[0]->entry();

assert ($e->status() === S60_CALENDAR_STATUS_TENTATIVE);
$e->set_status(S60_CALENDAR_STATUS_CONFIRMED);
$e->commit();

assert ($db->get_entry($e->id())->status() === S60_CALENDAR_STATUS_CONFIRMED);

$e->remove();

$i1 = $db->find_instances_day(1155500000);
assert(count($i1) === 0);

echo 'ok';

 ?>
--EXPECTF--
Warning: Bad status in %s.php on line 15
again..
ok
