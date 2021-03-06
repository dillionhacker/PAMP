--TEST--
Bug #27719 (mktime returns incorrect timestamp for dst days)
--INI--
error_reporting=2047
--FILE--
<?php /* $Id: bug27719.phpt,v 1.1 2005/07/03 14:36:59 derick Exp $ */
	putenv("TZ=EST");  // No DST
	$a = mktime(0, 0, 0, 4, 4, 2004, 0);
	$b = mktime(0, 0, 0, 4, 4, 2004, 1);
	$c = mktime(0, 0, 0, 4, 4, 2004, -1);
	echo "$a ".date("m/d/y h:i:s\n",$a);
	echo "$b ".date("m/d/y h:i:s\n",$b);
	echo "$c ".date("m/d/y h:i:s\n",$c);
	echo "\n";
	putenv("TZ=EST5EDT");  // DST not in effect
	$a = mktime(0, 0, 0, 2, 4, 2004, 0);
	$b = mktime(0, 0, 0, 2, 4, 2004, 1);
	$c = mktime(0, 0, 0, 2, 4, 2004, -1);
	echo "$a ".date("m/d/y h:i:s\n",$a);
	echo "$b ".date("m/d/y h:i:s\n",$b);
	echo "$c ".date("m/d/y h:i:s\n",$c);
	echo "\n";
	putenv("TZ=EST5EDT");  // Just before DST changeover
	$a = mktime(0, 0, 0, 4, 4, 2004, 0);
	$b = mktime(0, 0, 0, 4, 4, 2004, 1);
	$c = mktime(0, 0, 0, 4, 4, 2004, -1);
	echo "$a ".date("m/d/y h:i:s\n",$a);
	echo "$b ".date("m/d/y h:i:s\n",$b);
	echo "$c ".date("m/d/y h:i:s\n",$c);
	echo "\n";
	putenv("TZ=EST5EDT");  // Just after DST changeover
	$a = mktime(3, 0, 0, 4, 4, 2004, 0);
	$b = mktime(3, 0, 0, 4, 4, 2004, 1);
	$c = mktime(3, 0, 0, 4, 4, 2004, -1);
	echo "$a ".date("m/d/y h:i:s\n",$a);
	echo "$b ".date("m/d/y h:i:s\n",$b);
	echo "$c ".date("m/d/y h:i:s\n",$c);
	echo "\n";
	putenv("TZ=EST5EDT");  // DST in effect
	$a = mktime(0, 0, 0, 6, 4, 2004, 0);
	$b = mktime(0, 0, 0, 6, 4, 2004, 1);
	$c = mktime(0, 0, 0, 6, 4, 2004, -1);
	echo "$a ".date("m/d/y h:i:s\n",$a);
	echo "$b ".date("m/d/y h:i:s\n",$b);
	echo "$c ".date("m/d/y h:i:s\n",$c);
	echo "\n";
?>
--EXPECTF--
1081054800 04/04/04 12:00:00
%s
1081054800 04/04/04 12:00:00

1075870800 02/04/04 12:00:00
1075867200 02/03/04 11:00:00
1075870800 02/04/04 12:00:00

1081054800 04/04/04 12:00:00
1081051200 04/03/04 11:00:00
1081054800 04/04/04 12:00:00

1081065600 04/04/04 04:00:00
1081062000 04/04/04 03:00:00
1081062000 04/04/04 03:00:00

1086325200 06/04/04 01:00:00
1086321600 06/04/04 12:00:00
1086321600 06/04/04 12:00:00
