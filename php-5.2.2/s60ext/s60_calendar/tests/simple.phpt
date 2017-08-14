--TEST--
Very simple s60_calendar test.
--FILE--
<?php

function my_assert_handler($file, $line, $code)
{
	$lines = file($file); 
	echo "\nFailure:".trim($lines[$line-1])."\n";
}
assert_options(ASSERT_CALLBACK, 'my_assert_handler');

$db = s60_calendar_open("ctest.db", "n");
$c = $db->add_entry(S60_CALENDAR_TYPE_MEETING);
$c->set_content("cont");
$c->set_location("loca");
$c->set_start_end_time(1155500000,1155600000);
$c->commit();

$i1 = $db->find_instances_day(1155500000);
$i2 = $db->find_instances_range(1155450000, 1155550000, "cont", S60_CALENDAR_FILTER_MEETING);
$i3 = $db->find_instances_month(1155500000);
$i4 = $db->find_instances_range(1155450000, 1155550000, "cont", S60_CALENDAR_FILTER_ANNIV);

assert($i1[0]->date() <= 1155500000 && $i1[0]->date() >= 1155400000);
assert($i2[0]->date() <= 1155500000 && $i2[0]->date() >= 1155400000);
assert($i3[0]->date() <= 1155500000 && $i3[0]->date() >= 1155400000);
assert(count($i4) === 0);

$e = $i1[0]->entry();

assert($e->content() === "cont");
assert($e->content() === $i2[0]->entry()->content());
assert($e->content() === $i3[0]->entry()->content());

assert($e->location() === "loca");
assert(abs($e->start_time() - 1155500000) < 30);
assert(abs($e->end_time() - 1155600000) < 30);
assert(abs($e->last_modified() - time()) < 3600*24);

assert($e->type() === S60_CALENDAR_TYPE_MEETING);
assert($e->replication() === S60_CALENDAR_REP_OPEN);
assert($e->alarm() === False);
assert($e->crossed_out() === False);
assert(@$e->priority() === Null || is_int($e->priority()));
assert($e->repeat() === False);
assert(@$e->status() === Null || $e->status() === S60_CALENDAR_STATUS_NULL);

$e->remove();

$i1 = $db->find_instances_day(1155500000);
assert(count($i1) === 0);

echo 'ok';

 ?>
--EXPECTF--
ok
