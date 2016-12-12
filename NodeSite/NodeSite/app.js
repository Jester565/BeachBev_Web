var fs = require('fs');
var http = require('http');

var express = require('express');
var app = express();

var localPort = 80;
var localAddress = "0.0.0.0";

app.use('/', function (req, res, next) {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow_Headers", "Origin, X-Requested-With, Content-Type, Accept");
  next();
});

app.use(express.static(__dirname + '/public'));

var webServer = app.listen(localPort, localAddress, function () {
  console.log("Running");
});

webServer.on('error', function (err) {
  console.log("ERROR: " + err);
});

webServer.on('uncaughtException', function (err) {
  console.log("UNCAUGHT EXCEPTION: " + err);
});