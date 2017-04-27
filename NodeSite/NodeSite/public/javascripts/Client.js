'use strict';

var client = null;
function Client(domain, port, protoInitCallback) {
	client = this;
	this._domain = domain;
	this._port = port;
	this._protoInitCallback = protoInitCallback;

	this.connect = function () {
		client.tcpConnection.connect(client._domain, client._port);
	}

	//Initializes the protobuf addon
	protobuf.load("PackFW.proto", function (err, root) {
		if (err) {
			throw err;
		}
		client.root = root;
		//Create the tcpConnection object
		client.tcpConnection = new TCPConnection(root);

		client.packetManager = new PacketManager();
		client.tcpConnection.onmessage = function (iPack) { client.packetManager.processPacket(iPack); };

		client._protoInitCallback(root);
		client.connect();
	});
}
