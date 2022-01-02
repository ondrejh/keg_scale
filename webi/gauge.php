<?php

// wifi config
$wssid = isset($_POST['wssid']) ? $_POST['wssid'] : null;
$wpwd = isset($_POST['wpwd']) ? $_POST['wpwd'] : null;
$kegip = isset($_POST['kegip']) ? $_POST['kegip'] : null;

// gauge calib
$g04 = isset($_POST['g04']) ? $_POST['g01'] : null;
$g14 = isset($_POST['g14']) ? $_POST['g14'] : null;
$g24 = isset($_POST['g24']) ? $_POST['g24'] : null;
$g34 = isset($_POST['g34']) ? $_POST['g34'] : null;
$g44 = isset($_POST['g44']) ? $_POST['g44'] : null;
$gtest = isset($_POST['gtest']) ? $_POST['gtest'] : null;
$gsave = isset($_POST['gsave']);
$gcancel = isset($_POST['gcancel']);

/* ---- functions ----------------------------------------------------*/

// get json data
function get_json() {
	$f = fopen("gauge.json", "r") or die("Unable to open file!");
	$sf = fread($f,filesize("gauge.json"));
	fclose($f);
	$obj = json_decode($sf, false);
	/*$u = array();
	$v = array();
	if ( isset($obj->{'calib'}) ) {
		$c = ($obj->{'calib'});
		$cv = get_object_vars($c);
		foreach ( $cv as $k => $vv ) {
			array_push( $u, floatval($k) );
			array_push( $v, $vv );
		}
	}
	$cal['units'] = $u;
	$cal['raw'] = $v;
	return $cal;*/
	return $obj;
}

/*// bubble sort calibration (from get_calib) according to raw/units 
function bubble_calib( $cal, $unit ) {
	$c = $cal;
	$l = sizeof( $c[$unit] );
	$k = array_keys( $c );
	for ($j = 1; $j < $l; $j ++) {
		for ($i = 1; $i < $l; $i ++) {
			if ( $c[$unit][$i] < $c[$unit][$i-1] ) {
				foreach ( $k as $kk ) {
					$v = $c[$kk][$i];
					$c[$kk][$i] = $c[$kk][$i-1];
					$c[$kk][$i-1] = $v;
				}
			}
		}
	}
	return $c;
}

// interpolate value using calibration table according to raw/units
function interpol( $cal, $value, $unit ) {
	$l = sizeof( $cal[$unit] ); # get table length
	if ($l < 1)
		return NULL; # if table len == 0 no interpolation possible

	# get searched key
	foreach ( array_keys( $cal ) as $k ) if ( $k != $unit ) break;
	if ($l < 2)
		return $cal[$k][0]; # if table len == 1 no interpolation needed

	# get sorted calibration table
	$c = bubble_calib( $cal, $unit );
	for ( $i = 1; $i < ($l - 1); $i ++ ) {
		if ( $value < $c[$unit][$i] )
			break;
	}
	
	# get slope
	$a = ($c[$k][$i] - $c[$k][$i-1]) / ($c[$unit][$i] - $c[$unit][$i-1]);
	return $a * ($value - $c[$unit][$i]) + $c[$k][$i];
}

// print callibration table to json string
function print_calib( $cal ) {
	$str = '"calib": {';
	$l = sizeof($cal['units']);
	for ( $i=0; $i<$l; $i++ ) {
		if ($i!=0) $str .= ', ';
		$str .= '"'. sprintf('%0.01f', $cal['units'][$i]). '": '. $cal['raw'][$i];
	}
	$str .= '}';
	return $str;
}

// read object from json file (return empty object if file not found)
function json_fopen( $fname ) {
	$sf = '{}';
	if ( file_exists( $fname ) ) {
		$f = fopen( $fname, 'r' );
		$sf = fread($f,filesize( $fname ));
		fclose($f);
	}

	$obj = json_decode($sf, false);
	return $obj;
}

// save json object into the file
function json_fsave( $fname, $jobj ) {
	$sf = json_encode($jobj);
	$f = fopen( $fname , 'w' );
	fwrite($f, $sf);
	fclose($f);
}

// add (or change) one root key - value in the json file
function add_json_key( $fname, $jkey, $jval ) {
	$jobj = json_fopen( $fname );
	$jobj->{$jkey} = $jval;
	json_fsave( $fname, $jobj );
}*/

function set_gauge( $value, $description ) {
	echo "<h1>Setting ". $description. "</h1>";
	if ( is_numeric( $value ) )
		echo "<p>Value: ". (int)$value. "</p>";
	else
		echo "<p>Wrong value ". $value. "</p>";
}

/* ------- end of functions ------------------------------------------*/
echo "<html><body>";

// wifi settings ( on click set wifi )
if ( $wssid != null ) {
	echo "<h1>Wifi setup</h1>";
	echo "<p>Wifi SSID: <b>". $wssid. "</b><br/>";
	echo "Password: ". (($wpwd != null) ? ("<b>". $wpwd. "</b>") : "<b>k3Gat0rr</b> (default)"). "<br/>";
	echo "Kegator IP: ". (($kegip != null) ? ("<b>". $kegip. "</b>") : "<b>192.168.42.1</b> (default)"). "<br/></p>";
}

// gauge value settings ( on change value )
if ( $g04 != null ) {
	set_gauge( $g04, "EMPTY" );
}
else if ( $g14 != null ) {
	set_gauge( $g14, "QUARTER" );
}
else if ( $g24 != null ) {
	set_gauge( $g24, "HALF" );
}
else if ( $g34 != null ) {
	set_gauge( $g34, "TREE QUARTERS" );
}
else if ( $g44 != null ) {
	set_gauge( $g44, "FULL" );
}

// start gauge testing ( on click radio button )
if ( $gtest != null ) {
	echo "<p>Test ";
	if ( $gtest == "g04" ) echo "EMPTY";
	else if ( $gtest == "g14" ) echo "LAST QUARTER";
	else if ( $gtest == "g24" ) echo "HALF";
	else if ( $gtest == "g34" ) echo "TREE QUARTERS";
	else if ( $gtest == "g44" ) echo "FULL";
	else echo "ERROR";
}

// save gauge setting ( on click save )
if ( $gsave ) {
	echo "<p>SAVE gauge value</p>";
}

// cancel gauge setting ( on click cancel )
if ( $gcancel ) {
	echo "<p>CANCEL gauge value</p>";
}

echo "</body></html>";
/*// new keg
if (( $keg != null ) && ( $vol != null )) {
	$v = floatval($vol);
	if ($v == 0) {
		$succ = false;
	}
	else {
		$cal = get_calib();
		$units = $vol * 0.54 + 5.5;
		$raw = (int)interpol( $cal, $units, 'units' );
		$jd = json_fopen( 'data.json' );
		$jd->{'keg'} = array('name'=>$keg, 'fullraw'=>$raw, 'volume'=>$vol, 'left'=>$vol);
		$jd->{'units'} = sprintf('%0.01f', $units);
		$jd->{'raw'} = $raw;
		json_fsave( 'data.json', $jd );
		$succ = true;
	}
}

// finish keg
else if ( $del == 'yes' ) {
	$cal = get_calib();
	$jd = json_fopen( 'data.json' );
	unset($jd->{'keg'});
	json_fsave( 'data.json', $jd );
	$succ = true;
}

// set wifi
else if ( $wssid != null ) {
	$jobj = json_fopen( 'conf.json' );
	$jobj->{'wssid'} = $wssid;
	$jobj->{'wpwd'} = ($wpwd != null)?false:true;
	json_fsave( $fname, $jobj );
	$succ = true;
}

// set device key
else if ( $dkey != null ) {
	add_json_key( 'data.json', 'dkey', $dkey );
	$succ = true;
}

// calibration table - add value
else if ( $addc != null ) {
	// add row
	$obj = json_fopen('data.json');
	$raw = ( $rawc != null) ? $rawc : $obj->{'raw'};
	$obj->{'calib'}->{sprintf("%0.02f", $addc)} = $raw;

	// sort
	$u = array();
	$v = array();
	$c = ($obj->{'calib'});
	$cv = get_object_vars($c);
	foreach ( $cv as $k => $vv ) {
		array_push( $u, floatval($k) );
		array_push( $v, $vv );
	}
	$cal['units'] = $u;
	$cal['raw'] = $v;
	$cal = bubble_calib($cal, 'units');
	$l = sizeof($cal['units']);
	echo $l. PHP_EOL;
	$obj->{'calib'} = array();
	for ($i=0; $i<$l;$i++)
		$obj->{'calib'}[sprintf("%0.02f", $cal['units'][$i])] = $cal['raw'][$i];

	// save
	json_fsave( 'data.json', $obj );
	$succ = true;
}

// calibration table - remove value
else if ( $delc != null ) {
	$d = intval($delc);
	$obj = json_fopen('data.json');

	// list but forget value to be removed 
	$u = array();
	$v = array();
	$c = ($obj->{'calib'});
	$cv = get_object_vars($c);
	foreach ( $cv as $k => $vv ) {
		array_push( $u, floatval($k) );
		array_push( $v, $vv );
	}
	$cal['units'] = $u;
	$cal['raw'] = $v;
	$l = sizeof($cal['units']);
	echo $l. PHP_EOL;
	$obj->{'calib'} = array();
	for ($i=0; $i<$l;$i++) {
		if ($i + 1 != $d)
			$obj->{'calib'}[sprintf("%0.02f", $cal['units'][$i])] = $cal['raw'][$i];
	}

	// save
	json_fsave( 'data.json', $obj );
	$succ = true;
}

if ($succ == true) {
	echo 'OK';
}
else {
	echo 'ERROR';

	/*
	# pring calibration
	echo PHP_EOL;
	$cal = get_calib();
	print_calib($cal);
	*/

	/*
	# test interpolation
	echo PHP_EOL. PHP_EOL;;
	$cal = get_calib();
	for ($i = -5; $i < 60; $i += 5) {
		$r = interpol( $cal, $i, 'units' );
		$u = interpol( $cal, $r, 'raw' );
		echo $i. " ". $r. " ". $u. PHP_EOL;
	}
	*/
//}
?>
