--TEST--
Test all s60_calendar entry types except TODO.
--FILE--
<?php

function my_assert_handler($file, $line, $code) 
{
	$lines = file($file); 
	echo "\nFailure:".trim($lines[$line-1])."\n";
}
assert_options(ASSERT_CALLBACK, 'my_assert_handler');

$db = s60_calendar_open("ctest.db", "n");

$eka1 = (s60_sysinfo_sw_platform() === 'EKA1');

// ADD ENTRIES
$c = $db->add_entry(S60_CALENDAR_TYPE_MEETING);
$c->set_content("meeting");
$c->set_location("meeting loca");
$c->set_start_end_time(1355500000,1355600000);
$c->commit();
$c->set_priority(100);
$c->set_replication(S60_CALENDAR_REP_OPEN);
$c->set_alarm(1355400000);
if ($eka1) $c->set_crossed_out(False);
$c->commit();

$c = $db->add_entry(S60_CALENDAR_TYPE_EVENT);
$c->set_content("event");
$c->set_location("event loca");
$c->set_start_end_time(1355500000,1355600000);
$c->set_replication(S60_CALENDAR_REP_PRIVATE);
$c->commit();
$c->set_priority(200);
if ($eka1) $c->set_crossed_out(False);
$c->commit();

$c = $db->add_entry(S60_CALENDAR_TYPE_ANNIV);
$c->set_content("anniversary");
$c->set_location("anniversary loca");
$c->set_start_end_time(1355500000,1355600000);
$c->set_replication(S60_CALENDAR_REP_RESTRICTED);
$c->set_alarm(1355400000);
if ($eka1) $c->set_crossed_out(True); //clears the alarm..
$c->set_priority(50);
$c->commit();

$c = $db->add_entry(S60_CALENDAR_TYPE_TODO);
$c->set_content("todo");
$c->set_location("todo loca");
$c->set_start_end_time(1355500000,1355600000);
$c->set_replication(S60_CALENDAR_REP_RESTRICTED);
$c->set_alarm(1355400000);
$c->set_crossed_out(1355450000); //clears the alarm..
$c->set_priority(75);
$c->commit();


// entries(id) test with incorrect id:
assert (count($db->entries(9876938)) === 0);

// VERIFY ENTRIES
$ids = array();
$x = 0;
foreach ($db->find_instances_day(1355500000) as $i) {
	$e = $i->entry();
	// get_entry(id) test:
	$e = $db->get_entry($e->id());
	// entries(id) test with correct id:
	$all = $db->entries($e->id());
	assert ($all[0]->id() == $e->id());
	$ids[] = $e->id();
	switch ($e->type()) {
	case S60_CALENDAR_TYPE_MEETING:
		$x |= 1;
		echo "Test ...\n";
		assert ($e->content() === "meeting");
		assert ($e->location() === "meeting loca");
		assert ($e->replication() === S60_CALENDAR_REP_OPEN);
		if ($eka1) assert ($e->crossed_out() === False);
		assert ($e->priority() === 100);
		assert (abs($e->start_time() - 1355500000) < 60);
		assert (abs($e->end_time() - 1355600000) < 60);
		assert (abs($e->alarm() - 1355400000) < 60);
		break;
	case S60_CALENDAR_TYPE_EVENT:
		$x |= 2;
		echo "Test ...\n";
		assert ($e->content() === "event");
		assert ($e->location() === "event loca");
		if ($eka1) assert ($e->crossed_out() === False);
		assert ($e->priority() === 200);
		assert (abs($e->start_time() - 1355500000) < 3600*24);
		assert (abs($e->end_time() - 1355600000) < 3600*24);
		assert ($e->alarm() === False);
		break;
	case S60_CALENDAR_TYPE_ANNIV:
		$x |= 4;
		echo "Test ...\n";
		assert ($e->content() === "anniversary");
		assert ($e->location() === "anniversary loca");
		if ($eka1) assert ($e->crossed_out() === True);
		assert ($e->priority() === 50);
		assert (abs($e->start_time() - 1355500000) < 3600*24);
		assert (abs($e->end_time() - 1355600000) < 3600*24);
		if ($eka1) assert ($e->alarm() === False);
		else assert (abs($e->alarm() - 1355400000) < 60);
		break;
	case S60_CALENDAR_TYPE_TODO:
		$x |= 8;
		echo "Test ...\n";
		assert ($e->content() === "todo");
		assert ($e->location() === "todo loca");
		assert ($e->priority() === 75);
		assert (abs($e->start_time() - 1355500000) < 3600*24);
		assert (abs($e->end_time() - 1355600000) < 3600*24);
		assert ($e->alarm() === False);
		assert (abs($e->crossed_out() - 1355450000) < 3600*24);
		break;
	default:
		echo "error\n";
	}
	assert ($e->content() === $db->get_entry($e->id())->content());
}
assert ($x === 15);


//entries() test:
foreach ($db->entries() as $e) {
	assert (array_search($e->id(), $ids) !== False);
}


// REMOVE ENTRIES
echo "remove\n";

var_dump(count($db->entries()));
$i = $db->find_instances_day(1355500000);
$i[1]->entry()->remove();
var_dump(count($db->entries()));
var_dump(count($db->find_instances_day(1355500000)));
$i[2]->entry()->remove();
var_dump(count($db->entries()));
var_dump(count($db->find_instances_day(1355500000)));
$i[3]->entry()->remove();
var_dump(count($db->entries()));
var_dump(count($db->find_instances_day(1355500000)));
$i[0]->entry()->remove();
var_dump(count($db->entries()));
var_dump(count($db->find_instances_day(1355500000)));

echo "ok";

 ?>
--EXPECTF--
Test ...
Test ...
Test ...
Test ...
remove
int(4)
int(3)
int(3)
int(2)
int(2)
int(1)
int(1)
int(0)
int(0)
ok
