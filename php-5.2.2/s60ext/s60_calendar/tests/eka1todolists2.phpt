--TEST--
Test todo list remove.
--FILE--
<?php

function my_assert_handler($file, $line, $code) 
{
	$lines = file($file); 
	echo "\nFailure:".trim($lines[$line-1])."\n";
}
assert_options(ASSERT_CALLBACK, 'my_assert_handler');

$db = s60_calendar_open("ctest.db", "n");

$t1 = $db->add_todo_list();
$t1->set_name("list1");
$t1id = $t1->id();

// ADD TODOS
$c = $db->add_entry(S60_CALENDAR_TYPE_TODO);
$c->set_content("todo1");
$c->set_start_end_time(1355500000, 1355500000);
$c->set_todo_list($db->get_todo_list($t1id));
$c->set_priority(100);
$c->commit();

$c = $db->add_entry(S60_CALENDAR_TYPE_TODO);
$c->set_content("todo2");
$c->set_start_end_time(1355500000, 1355500000);
$c->set_todo_list($db->get_todo_list($t1id));
$c->set_priority(200);
$c->commit();

var_dump(count($db->todo_lists()));
var_dump(count($db->entries()));

$t1->remove();

var_dump(count($db->todo_lists()));
var_dump(count($db->entries()));

// TODO entry no longer has a list so an error is shown
var_dump($c->todo_list());

$t = $db->todo_lists();

// One can still assign the TODO entry to another list
var_dump($c->set_todo_list($t[0]));
var_dump($c->todo_list()->name());

echo "ok";


 ?>
--EXPECTF--
int(2)
int(2)
int(1)
int(2)

Warning: Illegal TODO list id in %s.php on line 40
NULL
bool(true)
string(9) "%s"
ok
