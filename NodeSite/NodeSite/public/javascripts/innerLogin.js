"use strict";

function InnerLoginManager(client, root, onLogin) {
		innerLoginManager = this;
		innerLoginManager.onLogin = onLogin;
		innerLoginManager.PacketA2 = root.lookup("ProtobufPackets.PackA2");
		innerLoginManager.PacketA1 = root.lookup("ProtobufPackets.PackA1");

		client.tcpConnection.onopen = function () {
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
		}

		client.packetManager.addPKey(new PKey("A1", function (iPack) {
				var packA1 = innerLoginManager.PacketA1.decode(iPack.packData);
    if (packA1.pwdToken === null) {
      window.location = './login.html?' + window.location;
    }
    else {
						Cookies.set('pwdToken', packA1.pwdToken, { expires: 1, path: '/', domain: 'beachbevs.com', secure: true });
						Cookies.set('deviceID', packA1.deviceID, { path: '/', domain: 'beachbevs.com', secure: true });
						Cookies.set('eID', packA1.eID, { path: '/', domain: 'beachbevs.com', secure: true });
						if (innerLoginManager.onLogin !== undefined) {
								innerLoginManager.onLogin();
						}
				}
		}, innerLoginManager, "Logs in user"));
}