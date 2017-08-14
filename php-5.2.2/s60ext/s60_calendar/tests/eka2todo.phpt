--TEST--
Tests s60_calendar's EKA2-only todo features (completed and incompleted todo)
--FILE--
<?php

function my_assert_handler($file, $line, $code)
{
	$lines = file($file);
	echo "\nFailure:".trim($lines[$line-1])."\n";
}
assert_options(ASSERT_CALLBACK, 'my_assert_handler');

$db = s60_calendar_open("ctest.db", "n");

$c = $db->add_entry(S60_CALENDAR_TYPE_TODO);
$c->set_content("cont");
$c->set_location("loca");
$c->set_start_end_time(1155500000,1155500060);
$c->set_status(S60_CALENDAR_STATUS_TENTATIVE); //fail
echo "again..\n";
$c->set_status(S60_CALENDAR_STATUS_TODO_COMPLETED);
$c->commit();
$e = $db->entries();
$e = $e[0];

assert (count($db->find_instances_day(1155500000, S60_CALENDAR_FILTER_TODO)) === 0);

$es = $db->entries();
$e = $es[0];

assert ($e->status() === S60_CALENDAR_STATUS_TODO_COMPLETED);
$e->set_status(S60_CALENDAR_STATUS_TODO_IN_PROGRESS);
$e->commit();

assert ($db->get_entry($e->id())->status() === S60_CALENDAR_STATUS_TODO_IN_PROGRESS);

assert (count($db->find_instances_day(1155500000, S60_CALENDAR_FILTER_TODO)) === 1);

$e->remove();

$i1 = $db->find_instances_day(1155500000);
assert(count($i1) === 0);

echo 'ok';


 ?>
--EXPECTF--
Warning: Bad status for TODO in %s.php on line 16
again..
ok
