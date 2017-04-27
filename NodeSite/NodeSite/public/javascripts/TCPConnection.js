'use strict';

var MAX_SIZE_BYTES = 2;

this.CONNECTION_STATES = {
	CONNECTED: { val: 0, err: false, msg: "Connection established" },
	SEARCHING: { val: 1, err: false, msg: "Still searching..." },
	TIMED_OUT: { val: 2, err: true, msg: "Unable to find host or the handshake was unsuccessful" },
	INVALID_ARGS: { val: 3, err: true, msg: "The ip or port was invalid" },
	CLOSED: { val: 4, err: true, msg: "The connection was closed" }
};

function TCPConnection(root) {
	var tcpConnect = this;
	if (root === undefined) {
		tcpConnect.connectionState = CONNECTION_STATES.INVALID_ARGS;
		throw (new Error("root is undefined"));
	}
	this.connectionState = CONNECTION_STATES.SEARCHING;

	this.PackHeaderIn = root.lookup("ProtobufPackets.PackHeaderOut");
	this.PackHeaderOut = root.lookup("ProtobufPackets.PackHeaderIn");

	this.connect = function (domain, port) {
		if (typeof domain !== 'string' || typeof port !== 'string') {
			this.connectionState = CONNECTION_STATES.INVALID_ARGS;
			throw (new Error("IP and Port must both be strings"));
		}

		this.bigEndian = isBigEndian();

		tcpConnect.socket = new WebSocket(domain + ":" + port);
		tcpConnect.socket.binaryType = "arraybuffer";
		tcpConnect.socket.onopen = function () {
			tcpConnect.connectionState = CONNECTION_STATES.CONNECTED;
			if (tcpConnect.onopen !== undefined) {
				tcpConnect.onopen();
			}
			console.log("Connection opened");
		};
		tcpConnect.socket.onclose = function (evt) {
			this.connectionState = CONNECTION_STATES.CLOSED;
			if (tcpConnect.onclose !== undefined) {
				tcpConnect.onclose();
			}
			console.log("Connection Closed: " + evt.code + " - " + evt.reason);
		};
		this.socket.onmessage = function (evt) {
			var dataArr = new Uint8Array(evt.data);
			var headerSize = 0;
			if (!tcpConnect.bigEndian) {
				headerSize = Number(((dataArr[0] & 0xff) << 8) | (dataArr[1] & 0xff));
			}
			else {
				headerSize = Number(((dataArr[1] & 0xff) << 8) | (dataArr[0] & 0xff));
			}
			var headerPackArr = new Uint8Array(headerSize);
			var packArr = new Uint8Array(dataArr.byteLength - headerSize - MAX_SIZE_BYTES);
			for (var i = 0; i < headerPackArr.byteLength; i++) {
				headerPackArr[i] = dataArr[i + MAX_SIZE_BYTES];
			}
			for (var i = 0; i < packArr.byteLength; i++) {
				packArr[i] = dataArr[i + MAX_SIZE_BYTES + headerSize];
			}
			var headerPackIn = tcpConnect.PackHeaderIn.decode(headerPackArr);
			console.log("Packet with key: " + headerPackIn.locKey + " received");
			var iPack = new IPacket(headerPackIn.locKey, headerPackIn.sentFromID, packArr);
			if (tcpConnect.onmessage !== undefined) {
				tcpConnect.onmessage(iPack);
			}
			else {
				console.log(iPack.toString());
			}
		};
		this.socket.onerror = function (evt) {
			console.warn("Socket error: " + evt.data);
			this.connectionState = CONNECTION_STATES.TIMED_OUT;
			if (TCPConnection.onerror !== undefined) {
				TCPConnection.onerror();
			}
		};
		this.sendPack = function (oPack) {
			console.log("Sending pack with key: " + oPack.locKey);
			var packUintArr = oPack.packBuilder.encode(oPack.pack).finish();
			var packSize = packUintArr.length;

			var headerPack = this.PackHeaderOut.create({ serverRead: oPack.serverRead, locKey: oPack.locKey, sendToIDs: oPack.sendToIDs });
			var headerPackUintArr = this.PackHeaderOut.encode(headerPack).finish();
			var headerPackSize = headerPackUintArr.length;
			if (headerPackSize % 1 !== 0) {
				alert("headerSize was not an integer...");
			}
			var totalArr = new Uint8Array(MAX_SIZE_BYTES + headerPackSize + packSize);
			if (!this.bigEndian) {
				totalArr[0] = (headerPackSize >> 8) & 0xff;
				totalArr[1] = (headerPackSize) & 0xff;
			}
			else {
				totalArr[0] = (headerPackSize) & 0xff;
				totalArr[1] = (headerPackSize >> 8) & 0xff;
			}
			totalArr.set(headerPackUintArr, MAX_SIZE_BYTES);
			totalArr.set(packUintArr, MAX_SIZE_BYTES + headerPackSize);
			this.socket.send(totalArr.buffer);
		};
	};
}

function isBigEndian() {
	var a = new ArrayBuffer(4);
	var b = new Uint8Array(a);
	var c = new Uint32Array(a);
	b[0] = 0xa1;
	b[1] = 0xb2;
	b[2] = 0xc3;
	b[3] = 0xd4;
	if (c[0] === 0xd4c3b2a1) {
		return false;
	}
	return true;
}