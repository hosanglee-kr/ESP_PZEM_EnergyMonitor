<!DOCTYPE html>
<html>
    <head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
	<meta name="description" content="PZEM PowerMeter - samples chart" />
	<title>Espem PowerMeter - samples chart</title>

	<!-- amCharts javascript sources -->
	<script src="https://www.amcharts.com/lib/3/amcharts.js" type="text/javascript"></script>
	<script src="https://www.amcharts.com/lib/3/serial.js" type="text/javascript"></script>
	<script src="https://www.amcharts.com/lib/3/themes/black.js" type="text/javascript"></script>
	<script src="https://www.amcharts.com/lib/3/plugins/export/export.min.js"></script>
	<script src="http://www.amcharts.com/lib/3/plugins/dataloader/dataloader.min.js" type="text/javascript"></script>

	<link rel="stylesheet" href="https://www.amcharts.com/lib/3/plugins/export/export.css" type="text/css" media="all" />

	<!-- amCharts javascript code -->
	<script type="text/javascript">
var getJSON = function(url, ok, err) {
	var xhr = new XMLHttpRequest();
	xhr.open('get', url, true);
	xhr.responseType = 'json';
	xhr.onload = function() {
	    var status = xhr.status;
	    if (status == 200) {
		ok && ok(xhr.response);
	    } else {
		err && err(status);
	    }
	};
	xhr.send();
};

getJSON('?s=chart&id=samples&devid={$devid}',
    function(data) {
	AmCharts.makeChart("chartsamples", data);
    },
    function(status) {
	console.log('Error loading chart chartsamples');
    }
);
</script>
    </head>
    <body>
	<div id="chartsamples" style="width: 100%; height: 600px; background-color: #000000;" ></div>
    </body>
</html>