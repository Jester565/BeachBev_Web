var httpPort = 80;
var httpsPort = 443;
var localAddress = "0.0.0.0";

var fs = require('fs');
var http = require('http');

var https = require('https');

var mySQL = require('mysql');

var sqlConnection = mySQL.createConnection({
	host: 'jester-mysql.chlnegeve40o.us-west-2.rds.amazonaws.com',
	user: 'jester',
	password: "Makarov's1Dog!"
});

sqlConnection.connect(function (err) {
	if (err) {
		console.log("Error occured when connecting to the database " + err);
	}
});
var credentials;
var privateKey = fs.readFileSync('/etc/letsencrypt/live/beachbevs.com/privkey.pem');
var certificate = fs.readFileSync('/etc/letsencrypt/live/beachbevs.com/fullchain.pem');
var credentials = {
	key: privateKey, cert: certificate
};

var express = require('express');
var bodyParser = re
var app = express();

app.use(express.static(__dirname + '/public'));
app.use(bodyParser.json({ limit: '5mb' }));
app.use(bodyParser.urlencoded({ limit: '5mb' }));

app.post('/inLocation.html', function (request, response)) {
	var contype = request.headers['content-type'];
	if (!contype || contype.indexOf('application/json') !== 0) {
		var unixTime = Math.round((new Date()).getTime() / 1000);
		var eventMsg = "Employee has " + request.body.eoe;
		var post = { msg: eventMsg, time: unixTime };
		var query = sqlConnection.query('INSERT INTO Events SET ?', post, function (err, result) {
			if (err) {
				console.log("Insert error");
			}
		});
		return res.send(400);
	}
});

var httpsServer = https.createServer(credentials, app);

httpsServer.listen(httpsPort, localAddress, function () {
	console.log("HTTPS Running");
});

var httpServer = http.createServer(function (req, res) {
	res.writeHead(301, { "Location": "https://" + req.headers['host'] + req.url });
	res.end();
});

httpServer.listen(httpPort, localAddress, function () {
	console.log("HTTP RUNNING");
});