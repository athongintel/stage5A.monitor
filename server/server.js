var express = require('express');
var server = express();

var sys = require('sys');
var exec = require('child_process').exec;

var serviceRunning = false;
var liveTime = 0;


var child;

child = exec("pwd", function (err, stdout, stderr) {
	sys.print('stdout: ' + stdout);
	sys.print('stderr: ' + stderr);
	if (err !== null) {
		console.log('exec error: ' + err);
	}
});



//0. activate openVPN connection



//1. open server at port 6969
server.get('/', function(request, response) {
	console.log('Request at /');
	response.writeHead(200, {"Content-Type" : "application/json"});
	var json = {};
	json.status = serviceRunning;
	json.liveTime = liveTime;	
	response.end(JSON.stringify(json));
});

server.listen(80);

server.get('/startService', function(request, response){
	if (!serviceRunning){
		//	
	}
});

