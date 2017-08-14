--TEST--
Test all s60_calendar entry types except TODO, using EKA1 interface db::entries().
--FILE--
<?php

function my_assert_handler($file, $line, $code) 
{
	$lines = file($file); 
	echo "\nFailure:".trim($lines[$line-1])."\n";
}
assert_options(ASSERT_CALLBACK, 'my_assert_handler');

$db = s60_calendar_open("ctest.db", "n");

// ADD ENTRIES
$c = $db->add_entry(S60_CALENDAR_TYPE_MEETING);
$c->set_content("meeting");
$c->set_location("meeting loca");
$c->set_start_end_time(1355500000,1355600000);
$c->commit();
$c->set_replication(S60_CALENDAR_REP_OPEN);
$c->set_crossed_out(True);
$c->commit();

$c = $db->add_entry(S60_CALENDAR_TYPE_EVENT);
$c->set_content("event");
$c->set_location("event loca");
$c->set_start_end_time(1355500000,1355600000);
$c->set_replication(S60_CALENDAR_REP_PRIVATE);
$c->commit();
$c->set_crossed_out(False);
$c->commit();

$c = $db->add_entry(S60_CALENDAR_TYPE_ANNIV);
$c->set_content("anniversary");
$c->set_location("anniversary loca");
$c->set_start_end_time(1355500000,1355600000);
$c->set_replication(S60_CALENDAR_REP_RESTRICTED);
$c->set_crossed_out(True);
$c->commit();

// VERIFY ENTRIES

$x = 0;
foreach ($db->entries() as $e) {
	switch ($e->type()) {
	case S60_CALENDAR_TYPE_MEETING:
		$x |= 1;
		echo "Test meeting...\n";
		assert ($e->content() === "meeting");
		assert ($e->location() === "meeting loca");
		assert ($e->replication() === S60_CALENDAR_REP_OPEN);
		assert ($e->crossed_out() === True);
		assert(abs($e->start_time() - 1355500000) < 60);
		assert(abs($e->end_time() - 1355600000) < 60);
		break;
	case S60_CALENDAR_TYPE_EVENT:
		$x |= 2;
		echo "Test event...\n";
		assert ($e->content() === "event");
		assert ($e->location() === "event loca");
		assert ($e->crossed_out() === False);
		assert(abs($e->start_time() - 1355500000) < 3600*24);
		assert(abs($e->end_time() - 1355600000) < 3600*24);
		break;
	case S60_CALENDAR_TYPE_ANNIV:
		$x |= 4;
		echo "Test anniv...\n";
		assert ($e->content() === "anniversary");
		assert ($e->location() === "anniversary loca");
		assert ($e->crossed_out() === True);
		assert(abs($e->start_time() - 1355500000) < 3600*24);
		assert(abs($e->end_time() - 1355600000) < 3600*24);
		break;
	default:
		echo "error\n";
	}
}
assert ($x === 7);

// REMOVE ENTRIES

$es = $db->entries();
$es[1]->remove();
var_dump(count($db->entries()));
$es[2]->remove();
var_dump(count($db->entries()));
$es[0]->remove();
var_dump(count($db->entries()));

echo "ok";

 ?>
--EXPECTF--
Test meeting...
Test event...
Test anniv...
int(2)
int(1)
int(0)
ok
