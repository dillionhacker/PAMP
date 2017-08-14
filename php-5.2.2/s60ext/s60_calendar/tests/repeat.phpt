--TEST--
s60_calendar repeat test.
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

define('NOW', time());

function get_db_repeat($repeat) {
	$db = s60_calendar_open("ctest.db", "n");
	$c = $db->add_entry(S60_CALENDAR_TYPE_MEETING);
	$c->set_content("test_cont");
	$c->set_location("test_loca");
	$c->set_start_end_time(NOW, NOW + HOUR);
	$c->set_repeat($repeat);
	$c->set_repeat($c->repeat());
	$c->commit();
	return $db;
}


function repeat_daily() {
	echo "daily repeat\n";
	$db = get_db_repeat(array(
		"type"=>"daily",
		"start"=>NOW,
		"end"=>NOW + WEEK*2,
		"interval"=>2));
		
	// Assert that the repeated entry is found
	// every second day, total of 8 instances.
	$pwd = -1;
	$wd = 0;
	$count = 0;
	for ($t = NOW; $t < NOW+WEEK*3; $t += DAY) {
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

function repeat_daily_exceptions() {
	echo "daily repeat + exceptions\n";
	$db = get_db_repeat(array(
		"type"=>"daily",
		"start"=>NOW,
		"end"=>NOW + WEEK*2,
		"interval"=>2,
		"exceptions"=>array(NOW+DAY*4, NOW+DAY*6)));

	// Assert that the repeated entry is found
	// on specific days: 0,2,8,10,12,14
	$ds = array();
	$wd = 0;
	for ($t = NOW; $t < NOW+WEEK*3; $t += DAY) {
		if (count($db->find_instances_day($t)) !== 0) {
			$ds[] = $wd;
		}
		$wd = $wd + 1;
	}
	assert ($ds == array(0,2,8,10,12,14));
	echo "done\n";
}

function repeat_daily_remove() {
	echo "daily repeat + remove instances\n";
	$db = get_db_repeat(array(
		"type"=>"daily",
		"start"=>NOW,
		"end"=>NOW + WEEK*3,
		"interval"=>1));
		
	// Remove some past instances, some specific
	// day instances and some future instances
	$inst = $db->find_instances_day(NOW + DAY*2);
	$inst[0]->remove(S60_CALENDAR_REMOVE_PAST);

	$inst = $db->find_instances_day(NOW + DAY*4);
	$inst[0]->remove();
	
	$inst = $db->find_instances_day(NOW + DAY*6);
	$inst[0]->remove();

	$inst = $db->find_instances_day(NOW + DAY*12);
	$inst[0]->remove(S60_CALENDAR_REMOVE_FUTURE);
	
	// Assert that the repeated entry is found
	// on specific days: 3,5,7,8,9,10,11
	$ds = array();
	$wd = 0;
	for ($t = NOW; $t < NOW+WEEK*4; $t += DAY) {
		if (count($db->find_instances_day($t)) !== 0) {
			$ds[] = $wd;
		}
		$wd = $wd + 1;
	}
	assert ($ds == array(3,5,7,8,9,10,11));
	echo "done\n";
}

function repeat_weekly() {
	echo "weekly repeat\n";
	$db = get_db_repeat(array(
		"type"=>"weekly",
		"start"=>NOW,
		"end"=>NOW + WEEK*5,
		"interval"=>2,
		"days"=>array(5)));
	
	// Assert that the repeated entry is found
	// on 3 days, all the same weekday
	$pwd = -1;
	$wd = 0;
	$count = 0;
	for ($t = NOW; $t < NOW+WEEK*15; $t += DAY) {
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


function repeat_weekly_forever() {
	echo "weekly repeat forever\n";
	$db = get_db_repeat(array(
		"type"=>"weekly",
		"start"=>NOW,
		"interval"=>2,
		"days"=>array(5)));
	
	// Assert that the repeated entry is found
	// on 8 days, all the same weekday.
	// Just testing 15 weeks since that's almost forever..
	$pwd = -1;
	$wd = 0;
	$count = 0;
	for ($t = NOW; $t < NOW+WEEK*15; $t += DAY) {
		if (count($db->find_instances_day($t)) !== 0) {
			$count++;
			if ($pwd !== -1) { assert ($wd === $pwd); }
			$pwd = $wd;
		}
		$wd = ($wd + 1) % 7;
	}
	assert ($count === 8);
	echo "done\n";
}


function repeat_monthly_by_dates() {
	echo "monthly by dates repeat\n";
	$db = get_db_repeat(array(
		"type"=>"monthly_by_dates",
		"start"=>NOW,
		"end"=>NOW + WEEK*14,
		"days"=>array(0, 14),
		"interval"=>1));

	// Assert that the repeat entry has instances
	// only on days 1 and 15, and number of them between 5-8
	$count = 0;
	for ($t = NOW; $t < NOW+WEEK*22; $t += DAY) {
		if (count($db->find_instances_day($t)) !== 0) {
			$count++;
			assert (date("j", $t) == 1 || date("j", $t) == 15);
		}
	}
	assert ($count > 4 && $count < 9); // XXX I didn't really think this through
	echo "done\n";
}


function repeat_monthly_by_days() {
	// XXX test may fail...
	
	echo "monthly by days repeat\n";
	$db = get_db_repeat(array(
		"type"=>"monthly_by_days",
		"start"=>NOW,
		"end"=>NOW + WEEK*12 - DAY,
		"days"=>array(array("week"=>2, "day"=>4)),
		"interval"=>1));
	
	// Assert that the repeated entry is found
	// on fridays and there is at least 20 days
	// between each instance. 3 instances total.
	$pwd = -1;
	$wd = 0;
	$count = 0;
	for ($t = NOW; $t < NOW+WEEK*22; $t += DAY) {
		if (count($db->find_instances_day($t)) !== 0) {
			$count++;
			assert (date("w", $t) == 5);
			if ($pwd !== -1) { assert ($wd > $pwd+20); }
			$pwd = $wd;
		}
		$wd = $wd + 1;
	}
	assert ($count === 3);
	echo "done\n";
}

function repeat_yearly_by_date() {
	echo "yearly by date repeat\n";
	$db = get_db_repeat(array(
		"type"=>"yearly_by_date",
		"start"=>NOW-10,
		"end"=>NOW + WEEK*120,
		"interval"=>1));

	// Assert that the repeat entry has 3 instances,
	// on different years but on same days.
	$count = 0;
	$pt = -1;
	for ($st = NOW; $st < NOW+WEEK*220; $st += DAY*365) {
		for ($t = $st-DAY*4; $t < $st+DAY*4; $t += DAY) {
			if (count($db->find_instances_day($t)) !== 0) {
				$count++;
				if ($pt !== -1) { assert (date("j", $t) == date("j", $pt)); }
				$pt = $t;
				//assert (date("j", $t) == 1 || date("j", $t) == 15);
			}
		}
	}
	assert ($count === 3);
	echo "done\n";
}



function repeat_yearly_by_day() {
	echo "yearly by day repeat\n";
	$db = get_db_repeat(array(
		"type"=>"yearly_by_day",
		"start"=>NOW-10,
		"end"=>NOW + WEEK*120,
		"days"=>array("month"=>7, "week"=>-1, "day"=>0),
		"interval"=>1));

	// Assert that the repeat entry has 2-3 instances,
	// on different years but on same weekday and near the end of month.
	$count = 0;
	for ($st = NOW; $st < NOW+WEEK*220; $st += DAY*365) {
		for ($t = $st-DAY*30; $t < $st+DAY*30; $t += DAY) {
			if (count($db->find_instances_day($t)) !== 0) {
				$count++;
				assert ((int)date("N", $t) === 1);
				assert ((int)date("j", $t) > 22);
			}
		}
	}
	assert ($count >= 2 && $count <= 3);
	echo "done\n";
}


repeat_daily(); sleep(1);
repeat_daily_exceptions(); sleep(1);
repeat_daily_remove(); sleep(1);
repeat_weekly(); sleep(1);
repeat_monthly_by_dates(); sleep(1);
repeat_monthly_by_days(); sleep(1);
repeat_yearly_by_date(); sleep(1);
repeat_yearly_by_day(); sleep(1);

echo 'ok';

 ?>
--EXPECTF--
daily repeat
done
daily repeat + exceptions
done
daily repeat + remove instances
done
weekly repeat
done
monthly by dates repeat
done
monthly by days repeat
done
yearly by date repeat
done
yearly by day repeat
done
ok
