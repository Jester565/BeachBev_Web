function redirect(url) {
	if (client !== null && client.tcpConnection !== null) {
		client.tcpConnection.onclose = null;
	}
	document.location.href = url;
}

function HandleConnectServer() {
	HideNoServer();
}

function HandleNoServer() {
	ShowNoServer();
	client.connect();
	console.log("Attempting reconnection");
}

function Client(protoInitCallback) {
	var client = this;

	this.connect = function () {
		if (window.location.protocol !== "https:") {
			client.tcpConnection.connect("beachbevs.com", "8000");
		}
		else {
			client.tcpConnection.connect("beachbevs.com", "8443");
		}
	}

	protobuf.load("PackFW.proto", function (err, root) {
		if (err) {
			throw err;
		}
		client.root = root;
		client.tcpConnection = new TCPConnection(root);

		client.packetManager = new PacketManager();
		client.tcpConnection.onmessage = function (iPack) { client.packetManager.processPacket(iPack); };

		protoInitCallback(root);
		client.connect();
	});
}