<?php
// deliver json data with chart properties based on template

header('Content-Type: application/json;charset=utf-8');

$chartid = $_REQUEST['id'];

switch ($chartid) {
  case 'samples':
    $smarty->display('chartsamples.json.tpl');
    break;
  default:
    print '{}';
}
