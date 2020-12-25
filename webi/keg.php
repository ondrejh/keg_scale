<?php
$keg = isset($_GET['keg']) ? $_GET['keg'] : null;
$vol = isset($_GET['vol']) ? $_GET['vol'] : null;
$del = isset($_GET['del']) ? $_GET['del'] : null;

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

if (( $keg != null ) && ( $vol != null )) {
	$v = floatval($vol);
	if ($v == 0) {
		echo "Error: incorrect volume input";
	}
	else {
		echo "New keg ". $keg. " volume ". $vol;
		$cal = get_calib();
		$units = $vol * 0.54 + 5.5;
		$raw = interpol( $cal, $units, 'units' );
		$f = fopen('data.json', 'w');
		fwrite($f, '{"raw": '. $raw. ', "units": '. sprintf( '%0.01f', $units). ', "temp": 25.5, "traw": 408, "primary_unit": "kg", "secondary_unit": "piv", "ratio": 0.500, '. print_calib($cal). ', "keg": {"name": "'. $keg. '", "fullraw": '. $raw. ', "volume": '. $vol. ', "left": '. $vol. '}}');
		fclose($f);
	}
}

else if ( $del == 'yes' ) {
	$cal = get_calib();
	$units = 0.0;
	$raw = interpol( $cal, $units, 'units' );
	$f = fopen('data.json', 'w');
	fwrite($f, '{"raw": '. $raw. ', "units": '. sprintf( '%0.01f', $units). ', "temp": 25.5, "traw": 408, "primary_unit": "kg", "secondary_unit": "piv", "ratio": 0.500, '. print_calib($cal). '}');
	fclose($f);
	echo "Kill keg ...";
}

else {
	echo "Input arguments missing";
	
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
	$cal = get_calib();
	var_dump(print_calib($cal));
}
?>
