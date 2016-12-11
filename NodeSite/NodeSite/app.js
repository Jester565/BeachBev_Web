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

app.listen(localPort, localAddress);

console.log("Running");

