<?php

$beers = $_POST['data'];
$file = 'seznam.txt';

if(!file_exists($file)) { // pokud soubor neexistuje, vytvorim prazdny
    file_put_contents($file, null);
}

// ulozeni do souboru
file_put_contents($file, $beers);

?>