<?php


function generate_array(){
$out = array();
$db = s60_contacts_open();
foreach($db->contacts() as $c){
$fields=array();
foreach($c->fields() as $f){
$fields[$f->label()]=$f->value();
}
$out[$c->title()]=$fields;
}
$db->close();
return $out;
}




function print_send_inputs($c,$k){
	$a=0;
	if(array_key_exists('Mobile',$c)){
	print '<input type="checkbox" name="smsto'.$k.'" disabled="true" >SMS <br>'."\n";
	$a=1;
	}

	if(array_key_exists('E-mail',$c)){
	print '<input type="checkbox" name="emailto'.$k.'"  >Email <br>'."\n";
	$a=1;
	}
	
	if($a==0){
		print "No suitable contact information!";
		}
	
	}
function print_select_input($name){
	print '<input type="checkbox" name="'. $name .'" >'."\n";
	}

function array_find_key_fields($keys,$list){
	$out = array();
	foreach($keys as $k){
		if(array_key_exists($k,$list)){
			$out[]=$k;
			}
		}
	
	
	return $out;
	}

function make_get_link($val,$l){
                $node=$_GET['q'];
	return  '<a href="?q='.$node.'&'.$val.'='.$l.'">'.$l.'</a>';
	
	}


function parse_contact($key,$con){
	$keys = array_keys($con);
	print "<b>".$key."</b>";
	print "<table>";
	foreach($keys as $k){
		print "<tr>";
		print "<td>";
		print $k.' :';
		print "</td>";
		print "<td>";
		print $con[$k];
		print "</td>";
		print "</tr>";

		}
	
	print "</table>";
	
	
	
	}

function POST(){
	$keys = array_keys($_POST);
	$out = array();
	foreach($keys as $k){
		$out[str_replace('_',' ',$k)]=$_POST[$k];
		
		}
	return $out;
	}












$conns = generate_array();

$keys = array_keys($conns);

print '<div style="width:600px;">';

print '<div style="float:left;">';
print '<form method="post" >'."\n";
print '<input type="hidden" name="kolme" value="3" >'."\n";

print '<div style="overflow: auto; border:1px solid; width:200px; height:300px; " >'."\n";
print "<table> \n";


foreach($keys as $k){
	print "<tr>\n";
	print "<td>\n";
	print print_select_input($k);
	print "</td>\n";
	print "<td>\n";
	print make_get_link('sel',$k) . "\n";
	print "</td>\n";
	
	
	print "</tr>\n";
	}

print "</table> \n";
print '</div>';

print '<input type="submit" value="Select" >'."\n";

print '</form>'."\n";

print '</div>';


print '<div style="float:right; border:1px solid; padding:5px; width:380px; height:310px; overflow:auto; " >';

if(array_key_exists('sel',$_GET) && !array_key_exists('kolme',$_POST) && !array_key_exists('viisi',$_POST)){
	parse_contact($_GET['sel'],$conns[$_GET['sel']]);
	}

if ($_POST['viisi']=='5'){
	print "<pre>";
	$sends = array_keys(POST());
	$email_cons=array();
	foreach($sends as $s){
		$pos = strpos($s,'emailto');
		if(!($pos===false)){$email_cons[]=substr($s,$pos+7);}
		}
	foreach($email_cons as $s){
		$emails[] = $conns[$s]['E-mail'];
		}

	print_r($emails);
	print "</pre>";
	}


if($_POST['kolme']=='3'){
	print '<form method="post">';
	print '<input type="hidden" name="viisi" value="5">';
	
	$sels = array_find_key_fields($keys,POST());
	print '<table >';
	
	foreach($sels as $s){
		print '<tr >';
		print '<td >';
		print $s;
		print '</td><td>';
		print_send_inputs($conns[$s],$s);
		print '</td>';
		print '</tr>';
		}
	


	print '</table>';	

	print 'Message:<br><textarea rows="6" cols="40" name="msg"></textarea>';
	print '<br>';	
	print '<input type="submit" value="Send" >';
	print '<input type="button" value="Back" onclick="window.back()" >';
	
	print '</form>';
	}
print '</div>';

print '</div>';
?> 