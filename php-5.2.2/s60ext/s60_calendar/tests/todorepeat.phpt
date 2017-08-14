--TEST--
s60_calendar repeat test for TODO items.
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
	$c = $db->add_entry(S60_CALENDAR_TYPE_TODO);
	$c->set_content("test_cont");
	$c->set_location("test_loca");
	$c->set_start_end_time(time(), time() + HOUR);
	$c->set_repeat($repeat);
	$c->set_repeat($c->repeat());
	$c->commit();
	return $db;
}


function repeat_daily() {
	echo "daily repeat\n";
	$db = get_db_repeat(array(
		"type"=>"daily",
		"start"=>time(),
		"end"=>time() + WEEK*2 + DAY,
		"interval"=>2));
		
	// Assert that the repeated entry is found
	// every second day, total of 8 instances.
	$pwd = -1;
	$wd = 0;
	$count = 0;
	for ($t = time(); $t < time()+WEEK*3; $t += DAY) {
		if (count($db->find_instances_day($t)) !== 0) {
			$count++;
			if ($pwd !== -1) { assert ($wd === $pwd+2); }
			$pwd = $wd;
		}
		$wd = $wd + 1;
	}
	assert ($count === 8);
	echo "done\n";
}


function repeat_weekly() {
	echo "weekly repeat\n";
	$db = get_db_repeat(array(
		"type"=>"weekly",
		"start"=>time(),
		"end"=>time() + WEEK*5,
		"interval"=>2,
		"days"=>array(5)));
	
	// Assert that the repeated entry is found
	// on 3 days, all the same weekday
	$pwd = -1;
	$wd = 0;
	$count = 0;
	for ($t = time(); $t < time()+WEEK*15; $t += DAY) {
		if (count($db->find_instances_day($t)) !== 0) {
			$count++;
			if ($pwd !== -1) { assert ($wd === $pwd); }
			$pwd = $wd;
		}
		$wd = ($wd + 1) % 7;
	}
	assert ($count === 3);
	echo "done\n";
}

repeat_daily(); sleep(1);
repeat_weekly();

echo 'ok';

 ?>
--EXPECTF--
daily repeat
done
weekly repeat
done
ok
