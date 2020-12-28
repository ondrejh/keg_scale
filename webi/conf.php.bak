<?php

$wssid = isset($_GET['wssid']) ? $_GET['wssid'] : null;
$wpwd = isset($_GET['wpwd']) ? $_GET['wpwd'] : null;
$dkey = isset($_GET['dkey']) ? $_GET['dkey'] : null;

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

function json_fsave( $fname, $jobj ) {
	$sf = json_encode($jobj);
	$f = fopen( $fname , 'w' );
	fwrite($f, $sf);
	fclose($f);
}

function add_json_key( $fname, $jkey, $jval ) {
	$jobj = json_fopen( $fname );
	$jobj->{$jkey} = $jval;
	json_fsave( $fname, $jobj );
}

$succ = false;

if ( $wssid != null ) {
	add_json_key( 'conf.json', 'wssid', $wssid );
	$succ = true;
}

if ( $dkey != null ) {
	add_json_key( 'data.json', 'dkey', $dkey );
	$succ = true;
}

if ( $succ )
	echo 'OK';
else
	echo 'ERROR';

?>
