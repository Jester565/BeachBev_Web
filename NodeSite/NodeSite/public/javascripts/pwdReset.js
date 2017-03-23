"use strict";

function PwdResetManager(root) {
	pwdResetManager = this;
	pwdResetManager.PacketA6 = root.lookup("ProtobufPackets.PackA6");
	pwdResetManager.PacketA7 = root.lookup("ProtobufPackets.PackA7");
	pwdResetManager.PacketA8 = root.lookup("ProtobufPackets.PackA8");
	pwdResetManager.PacketA1 = root.lookup("ProtobufPackets.PackA1");

	client.packetManager.addPKey(new PKey("A1", function (iPack) {
		var packA1 = pwdResetManager.PacketA1.decode(iPack.packData);
		if (packA1.pwdToken === null || packA1.pwdToken.length <= 0) {
			pwdResetManager.bindButtons();
			pwdResetManager.setErrorMsg(packA1.msg);
		}
		else {
			Cookies.set('pwdToken', packA1.pwdToken, { expires: 1, path: '/', domain: 'beachbevs.com', secure: true });
			Cookies.set('deviceID', packA1.deviceID, { path: '/', domain: 'beachbevs.com', secure: true });
			Cookies.set('eID', packA1.eID, { path: '/', domain: 'beachbevs.com', secure: true });
			$('#checkmark').removeClass('hidden');
			$('#pwdResetDive').addClass('hidden');
			pwdResetManager.setMsg(packA1.msg);
		}
	}, pwdResetManager, "Gets the success of the login"));

	client.packetManager.addPKey(new PKey("A7", function (iPack) {
		var packA7 = pwdResetManager.PacketA7.decode(iPack.packData);
		if (packA7.success) {
			$('#loading').addClass('hidden');
			$('#pwdResetDiv').removeClass('hidden');
			pwdResetManager.bindButtons();
		}
		else {
			pwdResetManager.setErrorMsg(packA7.msg);
		}
	}, pwdResetManager, "Gets the success of the password reset email"));

	pwdResetManager.bindButtons = function () {
		$('#changeButton').removeClass('processing');
		$('#changeButton').click(function () {
			if ($('#pwd').val().length <= 0) {
				pwdResetManager.setErrorMsg("No password was entered");
			}
			else if ($('#pwdConfirm').val() !== $('#pwd').val()) {
				pwdResetManager.setErrorMsg("Confirmation does match password");
			}
			else {
				var packA8 = pwdResetManager.PacketA8.create({
					pwdResetToken: pwdResetManager.pwdResetToken,
					pwd: $('#pwd').val()
				});
				client.tcpConnection.sendPack(new OPacket("A8", true, [0], packA8, pwdResetManager.PacketA8));
				pwdResetManager.unbindButtons();
			}
		});
	};

	pwdResetManager.unbindButtons = function () {
		$('#msg').addClass('hidden');
		$('#changeButton').unbind('click');
		$('#changeButton').addClass('processing');
	};

	pwdResetManager.setMsg = function (msg) {
		$('#msg').text(msg);
		$('#msg').removeClass('error');
		$('#msg').removeClass('hidden');
	};

	pwdResetManager.setErrorMsg = function (msg) {
		$('#msg').text(msg);
		$('#msg').addClass('error');
		$('#msg').removeClass('hidden');
		$('#msg').focus();
		$('window').scrollTo($('#msg'));
	};

	var url = window.location.href;
	var questionI = url.indexOf('?');
	if (questionI !== -1) {
		pwdResetManager.pwdResetToken = url.substring(++questionI);
		var packA6 = pwdResetManager.PacketA6.create({
			pwdResetToken: pwdResetManager.pwdResetToken
		});
		client.tcpConnection.sendPack(new OPacket("A6", true, [0], packA6, pwdResetManager.PacketA6));
	}
	else {
		setErrorMsg('No password reset token in url');
	}
}

var pwdResetManager;

var client = new Client(function (root) {
	console.log("ON LOAD CALLED");
	client.tcpConnection.onopen = function () {
		pwdResetManager = new PwdResetManager(client.root);
	};
	client.tcpConnection.onclose = function () {
		redirect('./noServer.html');
	};
});