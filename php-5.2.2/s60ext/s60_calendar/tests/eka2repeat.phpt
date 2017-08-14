--TEST--
Tests s60_calendar's EKA2-only repeat (rdates)
--FILE--
<?php

function my_assert_handler($file, $line, $code)
{
	$lines = file($file);
	echo "\nFailure:".trim($lines[$line-1])."\n";
}
assert_options(ASSERT_CALLBACK, 'my_assert_handler');

define('HOUR', 3600);
define('DAY', HOUR*24);
define('WEEK', DAY*7);


function get_db_repeat($repeat) {
	$db = s60_calendar_open("ctest.db", "n");
	$c = $db->add_entry(S60_CALENDAR_TYPE_MEETING);
	$c->set_content("test_cont");
	$c->set_location("test_loca");
	$c->set_start_end_time(time(), time() + HOUR);
	$c->set_repeat($repeat);
	var_dump($c->repeat());
	$c->set_repeat($c->repeat()); // check R/W ok
	$c->commit();
	return $db;
}

function repeat_rdates1() {
	echo "rdates repeat\n========================\n";
	$db = get_db_repeat(array(
		"type"=>"no_repeat",
		"rdates"=>array(time(), time() + DAY*3, time() + DAY*4, time() + DAY*7)));
	// Assert that the repeated entry is found
	// on the specified days
	$ds = array();
	$wd = 0;
	for ($t = time(); $t < time()+WEEK*4; $t += DAY) {
		if (count($db->find_instances_day($t)) !== 0) {
			$ds[] = $wd;
		}
		$wd = $wd + 1;
	}
	assert ($ds == array(0,3,4,7));
	echo "done\n\n\n";
}

function repeat_rdates2() {
	echo "rdates repeat weekly\n========================\n";
	$db = get_db_repeat(array(
		"type"=>"weekly",
		"start"=>time(),
		"rdates"=>array(time(), time() + DAY*3, time() + DAY*4, time() + DAY*7)));
	// Assert that the repeated entry is found
	// on the specified rdays and also weekly (0, 7, 14, ...).
	$ds = array();
	$wd = 0;
	for ($t = time(); $t < time()+WEEK*5-DAY; $t += DAY) {
		if (count($db->find_instances_day($t)) !== 0) {
			$ds[] = $wd;
		}
		$wd = $wd + 1;
	}
	assert ($ds == array(0,3,4,7,14,21,28));
	echo "done\n\n\n";
}

repeat_rdates1(); sleep(1);
repeat_rdates2();

echo 'ok';

 ?>
--EXPECTF--
rdates repeat
========================
array(2) {
  ["type"]=>
  string(9) "no_repeat"
  ["rdates"]=>
  array(4) {
    [0]=>
    float(%f)
    [1]=>
    float(%f)
    [2]=>
    float(%f)
    [3]=>
    float(%f)
  }
}
done


rdates repeat weekly
========================
array(5) {
  ["type"]=>
  string(6) "weekly"
  ["start"]=>
  float(%f)
  ["interval"]=>
  int(1)
  ["days"]=>
  array(1) {
    [0]=>
    int(%d)
  }
  ["rdates"]=>
  array(4) {
    [0]=>
    float(%f)
    [1]=>
    float(%f)
    [2]=>
    float(%f)
    [3]=>
    float(%f)
  }
}
done


ok