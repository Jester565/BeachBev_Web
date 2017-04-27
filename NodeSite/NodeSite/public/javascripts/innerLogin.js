'use strict';

var innerLoginManager = this;
function InnerLoginManager(client, root, onLogin, onFail) {
	innerLoginManager = this;
	this._onLogin = onLogin;
	if (onFail !== null) {
		this._onFail = onFail;
	}
	else
	{
		this._onFail = function (loginErr) {
			Redirect('./login.html?' + document.location.href);
		}
	}
	this.PacketA2 = root.lookup("ProtobufPackets.PackA2");
	this.PacketA9 = root.lookup("ProtobufPackets.PackA9");

	this.handleOpen = function () {
		var cookieEID = Cookies.get('eID');
		var cookieDevID = Cookies.get('deviceID');
		var cookiePwdToken = Cookies.get('pwdToken');
		if (cookieEID !== null && cookieDevID !== null && cookiePwdToken !== null) {
			var packA2 = innerLoginManager.PacketA2.create({
				eID: cookieEID,
				pwdToken: cookiePwdToken,
				deviceID: cookieDevID
			});
			client.tcpConnection.sendPack(new OPacket("A2", true, [0], packA2, innerLoginManager.PacketA2));
		}
		else
		{
			innerLoginManager._onFail();
		}
	};

	client.packetManager.addPKey(new PKey("A9", function (iPack) {
		var packA1 = innerLoginManager.PacketA1.decode(iPack.packData);
		if (packA1.pwdToken === null || packA1.pwdToken.length <= 0) {
			innerLoginManager._onFail();
		}
		else {
			Cookies.set('pwdToken', packA1.pwdToken, { expires: 1, path: '/', domain: window.location.hostname, secure: true });
			Cookies.set('deviceID', packA1.deviceID, { path: '/', domain: window.location.hostname, secure: true });
			Cookies.set('eID', packA1.eID, { path: '/', domain: window.location.hostname, secure: true });
			if (innerLoginManager._onLogin !== undefined) {
				innerLoginManager._onLogin();
			}
		}
	}, innerLoginManager, "Logs in user"));
}