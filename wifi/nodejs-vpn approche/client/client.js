var express = require('express');
var server = express();

//var sys = require('sys');
//var exec = require('child_process').exec;

var serviceRunning = false;
var liveTime = 0;


/*var child;

child = exec("pwd", function (err, stdout, stderr) {
	sys.print('stdout: ' + stdout);
	sys.print('stderr: ' + stderr);
	if (err !== null) {
		console.log('exec error: ' + err);
	}
});

*/

//0. activate openVPN connection




//1. open server at port 6969
server.get('/', function(request, response) {
	console.log('Request at /');
	
	var json = {};
	json.status = serviceRunning;
	json.liveTime = liveTime;

	response.writeHead(200, {"Content-Type" : "application/json"});
	response.end(JSON.stringify(json));
});

server.get('/start', function(request, response) {
	console.log('Request at /start');
	var json = {};
	if (serviceRunning){		
		json.error = true;
		json.errorReason = "Service already started";
	}
	else{
		json.error = false;
		//start iperf, stream down
		//or show average debit, jitter
		//
		serviceRunning = true;
	}
	response.writeHead(200, {"Content-Type" : "application/json"});
	response.end(JSON.stringify(json));
});

server.get('/stop', function(request, response) {
	console.log('Request at /stop');
	
	var json = {};
	if (!serviceRunning){		
		json.error = true;
		json.errorReason = "Service not started";
	}
	else{
		json.error = false;
	}
	
	response.writeHead(200, {"Content-Type" : "application/json"});
	response.end(JSON.stringify(json));
});

server.listen(6969, "0.0.0.0", 511, function(){
	console.log('Listening at 6969...');
});

