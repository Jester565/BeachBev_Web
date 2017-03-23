function redirect(url) {
	if (client !== null && client.tcpConnection !== null) {
		client.tcpConnection.onclose = null;
	}
	document.location.href = url;
}

function Client(protoInitCallback) {
	var client = this;
	protobuf.load("PackFW.proto", function (err, root) {
		if (err) {
			throw err;
		}
		client.root = root;
		client.tcpConnection = new TCPConnection(root);

		client.packetManager = new PacketManager();
		client.tcpConnection.onmessage = function (iPack) { client.packetManager.processPacket(iPack); };

		protoInitCallback(root);

		if (window.location.protocol !== "https:") {
			client.tcpConnection.connect(location.host, "8000");
		}
		else {
			client.tcpConnection.connect(location.host, "8443");
		}
	});
}