'use strict';

var setman = null;
var pwdResetManager = null;

$('document').ready(function () {
	$("#mBar").load("./mBar.html", function () {
		pwdResetManager = new PwdResetManager();
		setman = new SetupManager(false, pwdResetManager);
	});
});

function PwdResetManager() {
	pwdResetManager = this;

	this.onProto = function () {
		pwdResetManager.initPacks();
	}

	this.onOpen = function () {
		pwdResetManager.sendA6();
	}

	this.onReopen = function () {
		if (!$('#loading').hasClass('hidden')) {
			pwdResetManager.sendA6();
		}
		else if (pwdResetManager.bind()) {
			pwdResetManager.setErrorMsg("Request Failed: Connection Lost");
		}
	}

	this.initPacks = function () {
		pwdResetManager.PacketA6 = setman.client.root.lookup("ProtobufPackets.PackA6");
		pwdResetManager.PacketA7 = setman.client.root.lookup("ProtobufPackets.PackA7");
		pwdResetManager.PacketA8 = setman.client.root.lookup("ProtobufPackets.PackA8");
		pwdResetManager.PacketA1 = setman.client.root.lookup("ProtobufPackets.PackA1");

		setman.client.packetManager.addPKey(new PKey("A1", function (iPack) {
			var packA1 = pwdResetManager.PacketA1.decode(iPack.packData);
			if (packA1.pwdToken === null || packA1.pwdToken.length <= 0) {
				pwdResetManager.bindButtons();
				pwdResetManager.setErrorMsg(packA1.msg);
			}
			else {
				Cookies.set('pwdToken', packA1.pwdToken, { expires: 1, path: '/', domain: window.location.hostname, secure: true });
				Cookies.set('deviceID', packA1.deviceID, { path: '/', domain: window.location.hostname, secure: true });
				Cookies.set('eID', packA1.eID, { path: '/', domain: window.location.hostname, secure: true });
				$('#checkmark').removeClass('hidden');
				$('#pwdResetDiv').addClass('hidden');
				pwdResetManager.setMsg(packA1.msg);
			}
		}, pwdResetManager, "Gets the success of the login"));

		setman.client.packetManager.addPKey(new PKey("A7", function (iPack) {
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
	}

	this.bindButtons = function () {
		pwdResetManager.changeButton.removeClass('processing');
		pwdResetManager.changeButton.bind();
	};

	this.unbindButtons = function () {
		$('#msg').addClass('hidden');
		pwdResetManager.changeButton.unbind();
		pwdResetManager.changeButton.addClass('processing');
	};

	this.setMsg = function (msg) {
		$('#msg').text(msg);
		$('#msg').removeClass('error');
		$('#msg').removeClass('hidden');
	};

	this.setErrorMsg = function (msg) {
		$('#msg').text(msg);
		$('#msg').addClass('error');
		$('#msg').removeClass('hidden');
		$('#msg').focus();
		$('html, body').scrollTo($('#msg'), 100);
	};

	this.sendA6 = function () {
		var url = window.location.href;
		var questionI = url.indexOf('?');
		if (questionI !== -1) {
			pwdResetManager.pwdResetToken = url.substring(++questionI);
			var packA6 = pwdResetManager.PacketA6.create({
				pwdResetToken: pwdResetManager.pwdResetToken
			});
			setman.client.tcpConnection.sendPack(new OPacket("A6", true, [0], packA6, pwdResetManager.PacketA6));
		}
		else {
			pwdResetManager.setErrorMsg('No password reset token in url');
		}
	}

	this.changeButton = new PKeyButton('#changeButton', function () {
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
			setman.client.tcpConnection.sendPack(new OPacket("A8", true, [0], packA8, pwdResetManager.PacketA8));
			pwdResetManager.unbindButtons();
		}
	});
}
