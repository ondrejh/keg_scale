<?php
$keg = isset($_GET['keg']) ? $_GET['keg'] : null;
$vol = isset($_GET['vol']) ? $_GET['vol'] : null;
$del = isset($_GET['del']) ? $_GET['del'] : null;

if (( $keg != null ) && ( $vol != null )) {
	$v = floatval($vol);
	if ($v == 0) {
		echo "Error: incorrect volume input";
	}
	else {
		echo "New keg ". $keg. " volume ". $vol;
		$f = fopen('data.json', 'w');
		fwrite($f, '{"raw": -20306465, "units": 34.1, "temp": 25.5, "traw": 408, "primary_unit": "kg", "secondary_unit": "piv", "ratio": 0.500, "calib": {"30.0": -18753708, "10.0": -11127584, "0.0": -7110488}, "keg": {"name": "'. $keg. '", "fullraw": -20308667, "volume": '. $vol. ', "left": '. $vol. '}}');
		fclose($f);
	}
}

else if ( $del == 'yes' ) {
	$f = fopen('data.json', 'w');
	fwrite($f, '{"raw": -20306465, "units": 34.1, "temp": 25.5, "traw": 408, "primary_unit": "kg", "secondary_unit": "piv", "ratio": 0.500, "calib": {"30.0": -18753708, "10.0": -11127584, "0.0": -7110488}}');
	fclose($f);
	echo "Kill keg ...";
}

else {
	echo "Input arguments missing";
}
?>
