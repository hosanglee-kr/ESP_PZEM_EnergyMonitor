<?php

require_once( 'inc/local.php' );

$section = $_REQUEST['s'];

switch ($section) {
    case 'chart':
	require_once('inc/getchart.php');
	break;
    case 'psamples':
	$smarty->display('psamples.tpl');
	break;
    default:
	require_once( 'inc/pmeter.php' );
}

?>