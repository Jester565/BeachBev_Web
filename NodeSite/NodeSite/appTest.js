var httpPort = 80;
var httpsPort = 443;
var localAddress = "0.0.0.0";

var fs = require('fs');
var http = require('http');

var https = require('https');
var credentials;
var privateKey = fs.readFileSync('C:\\Users\\ajcra\\Desktop\\aws\\SSL\\local.key');
var certificate = fs.readFileSync('C:\\Users\\ajcra\\Desktop\\aws\\SSL\\local.crt');
var credentials = {
  key: privateKey, cert: certificate
};

var express = require('express');
var app = express();

app.use('/', function (req, res, next) {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow_Headers", "Origin, X-Requested-With, Content-Type, Accept");
  next();
});

app.use(express.static(__dirname + '/public'));


var httpsServer = https.createServer(credentials, app);

httpsServer.listen(httpsPort, localAddress, function () {
  console.log("HTTPS Running");
});

/*
var httpServer = http.createServer(app);

httpServer.get('*', function (req, res) {
  res.redirect('https://localhost' + req.url);
});

httpServer.listen(httpPort, localAddress, function () {
  console.log("HTTP RUNNING");
});
*/