--TEST--
Test todos and todo lists.
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

$t2 = $db->add_todo_list();
$t2->set_name("list2");
$t2id = $t2->id();

unset($t1);
unset($t2);

assert ($db->get_todo_list($t1id)->name() === "list1");
assert ($db->get_todo_list($t2id)->name() === "list2");
assert (@$db->get_todo_list(666) === Null);

$tls = $db->todo_lists();
assert ($tls[1]->id() === $t1id);
assert ($tls[2]->id() === $t2id);

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
$c->set_todo_list($db->get_todo_list($t2id));
$c->set_priority(200);
$c->commit();

$c = $db->add_entry(S60_CALENDAR_TYPE_TODO);
$c->set_content("todo3");
$c->set_start_end_time(1355500000, 1355500000);
$c->set_todo_list($db->get_todo_list($t1id));
$c->commit();

// now todo1 and todo3 are in list1, and todo2 in list2

$tds = $c->todo_list()->todos();
assert ($tds[0]->entry()->content() === "todo1");
assert ($tds[1]->entry()->content() === "todo3");
$tds = $db->get_todo_list($t2id)->todos();
assert ($tds[0]->entry()->content() === "todo2");

// move todo2 to list1 too
$e = $tds[0]->entry();
$e->set_todo_list($db->get_todo_list($t1id));
$e->commit();

// assert that list1 has 3 elements, list2 has 0
assert (count($db->get_todo_list($t1id)->todos()) === 3);
assert (count($db->get_todo_list($t2id)->todos()) === 0);

// verify priorities
foreach ($db->get_todo_list($t1id)->todos() as $t) {
	var_dump($t->entry()->priority());
}

// verify ids
foreach ($db->get_todo_list($t1id)->todos() as $t) {
	assert ($t->entry()->content() === $db->get_entry($t->entry()->id())->content());
}

$e->remove(); //remove entry
assert (count($db->get_todo_list($t1id)->todos()) === 2);
$tds = $c->todo_list()->todos();
$tds[0]->remove(); //remove instance
assert (count($db->get_todo_list($t1id)->todos()) === 1);


echo "ok";

 ?>
--EXPECTF--
int(100)
int(0)
int(200)
ok

