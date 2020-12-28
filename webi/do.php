<?php

// keg
$keg = isset($_POST['keg']) ? $_POST['keg'] : null;
$vol = isset($_POST['vol']) ? $_POST['vol'] : null;
$del = isset($_POST['del']) ? $_POST['del'] : null;

// config
$wssid = isset($_POST['wssid']) ? $_POST['wssid'] : null;
$wpwd = isset($_POST['wpwd']) ? $_POST['wpwd'] : null;
$dkey = isset($_POST['dkey']) ? $_POST['dkey'] : null;
//$npin = isset($_POST['npin']) ? $_POST['npin'] : null; future feature

// calib
$addc = isset($_POST['addc']) ? $_POST['addc'] : null;
$delc = isset($_POST['delc']) ? $_POST['delc'] : null;
$rawc = isset($_POST['valc']) ? $_POST['valc'] : null;

// pin
//$pin = isset($_POST['pin']) ? $_POST['pin'] : null; future feature

/* ---- functions ----------------------------------------------------*/

// get calibration from data.json
function get_calib() {
	$f = fopen("data.json", "r") or die("Unable to open file!");
	$sf = fread($f,filesize("data.json"));
	fclose($f);
	$obj = json_decode($sf, false);
	$u = array();
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
	return $cal;
}

// bubble sort calibration (from get_calib) according to raw/units 
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
}

/* ------- end of functions ------------------------------------------*/

$succ = false;

// new keg
if (( $keg != null ) && ( $vol != null )) {
	$v = floatval($vol);
	if ($v == 0) {
		$succ = false;
		//echo "Error: incorrect volume input";
	}
	else {
		$succ = true;
		//echo "New keg ". $keg. " volume ". $vol;
		$cal = get_calib();
		$units = $vol * 0.54 + 5.5;
		$raw = (int)interpol( $cal, $units, 'units' );
		$f = fopen('data.json', 'w');
		fwrite($f, '{"raw": '. $raw. ', "units": '. sprintf( '%0.01f', $units). ', "temp": 25.5, "traw": 408, "primary_unit": "kg", "secondary_unit": "piv", "ratio": 0.500, '. print_calib($cal). ', "keg": {"name": "'. $keg. '", "fullraw": '. $raw. ', "volume": '. sprintf( '%0.01f', $vol ). ', "left": '. sprintf( '%0.01f', $vol ). '}}');
		fclose($f);
	}
}

// finish keg
if ( $del == 'yes' ) {
	$cal = get_calib();
	$units = 0.0;
	$raw = (int)interpol( $cal, $units, 'units' );
	$f = fopen('data.json', 'w');
	fwrite($f, '{"raw": '. $raw. ', "units": '. sprintf( '%0.01f', $units). ', "temp": 25.5, "traw": 408, "primary_unit": "kg", "secondary_unit": "piv", "ratio": 0.500, '. print_calib($cal). '}');
	fclose($f);
	//echo "Kill keg ...";
	$succ = true;
}

// set wifi
if ( $wssid != null ) {
	$jobj = json_fopen( 'conf.json' );
	$jobj->{'wssid'} = $wssid;
	$jobj->{'wpwd'} = ($wpwd != null)?false:true;
	json_fsave( $fname, $jobj );
	$succ = true;
}

// set device key
if ( $dkey != null ) {
	add_json_key( 'data.json', 'dkey', $dkey );
	$succ = true;
}

// calibration table - add value
if ( $addc != null ) {
	// add row
	$obj = json_fopen('data.json');
	$raw = ( $rawc != null) ? $rawc : $obj->{'raw'};
	$obj->{'calib'}->{sprintf("%0.01f", $addc)} = $raw;

	// sort
	$u = array();
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
	$cal = bubble_calib($cal, 'units');
	$l = sizeof($cal['units']);
	echo $l. PHP_EOL;
	$obj->{'calib'} = array();
	for ($i=0; $i<$l;$i++)
		$obj->{'calib'}[sprintf("%0.01f", $cal['units'][$i])] = $cal['raw'][$i];

	// save
	json_fsave( 'data.json', $obj );
	$succ = true;
}

if ($succ == true) {
	echo 'OK';
}
else {
	echo 'ERROR';

	# pring calibration
	echo PHP_EOL;
	$cal = get_calib();
	print_calib($cal);

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
}
?>
