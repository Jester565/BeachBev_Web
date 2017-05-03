'use strict';

var setman = null;
var loginManager = null;

$('document').ready(function () {
	$("#mBar").load("./mBar.html", function () {
		loginManager = new LoginManager();
		setman = new SetupManager(false, loginManager);
	});
});

function LoginManager() {
	loginManager = this;

	this.onProto = function () {
		loginManager.initPacks();
	}

	this.onOpen = function () {
		loginManager.bindButtons();
		$('#loginDiv').removeClass('hidden');
		$('#loading').addClass('hidden');
	}

	this.onReopen = function () {
		if ($('#cancelButton').hasClass('processing')) {
			loginManager.bindButtons();
			loginManager.setErrorMsg("Request Failed: Connection Lost");
		}
	}

	this.initPacks = function () {
		loginManager.PacketA3 = setman.client.root.lookup("ProtobufPackets.PackA3");
		loginManager.PacketA1 = setman.client.root.lookup("ProtobufPackets.PackA1");
		loginManager.PacketA4 = setman.client.root.lookup("ProtobufPackets.PackA4");
		loginManager.PacketA5 = setman.client.root.lookup("ProtobufPackets.PackA5");

		setman.client.packetManager.addPKey(new PKey("A1", function (iPack) {
			var packA1 = loginManager.PacketA1.decode(iPack.packData);
			if (packA1.pwdToken === null || packA1.pwdToken.length <= 0) {
				loginManager.bindButtons();
				loginManager.setErrorMsg(packA1.msg);
			}
			else {
				console.log("EID: " + packA1.eID);
				console.log("WinLoc: " + window.location.hostname);
				Cookies.set('pwdToken', packA1.pwdToken, { expires: 1, path: '/', domain: window.location.hostname, secure: true });
				Cookies.set('deviceID', packA1.deviceID, { path: '/', domain: window.location.hostname, secure: true });
				Cookies.set('eID', packA1.eID, { path: '/', domain: window.location.hostname, secure: true });
				var url = document.location.href;
				var questionI = url.indexOf('?');
				if (questionI !== -1) {
					Redirect(url.substring(++questionI));
				}
				else {
					Redirect('./employee.html');
				}
			}
		}, loginManager, "Gets the success of the login"));

		setman.client.packetManager.addPKey(new PKey("A5", function (iPack) {
			loginManager.bindButtons();
			var packA5 = loginManager.PacketA5.decode(iPack.packData);
			if (packA5.success) {
				loginManager.setMsg(packA5.msg);
			}
			else {
				loginManager.setErrorMsg(packA5.msg);
			}
		}, loginManager, "Gets the success of the password reset email"));
	}

	this.bindButtons = function () {
		$('#loginButton').removeClass('processing');
		$('#pwdResetButton').removeClass('processing');
		$('#cancelButton').removeClass('processing');
		$('#loginButton').click(function () {
			if ($('#userName').val().length <= 0) {
				loginManager.setErrorMsg("No username was entered");
			}
			else if ($('#pwd').val().length <= 0) {
				loginManager.setErrorMsg("No password was entered");
			}
			else {
				var devID = Cookies.get('deviceID');
				if (devID === null) {
					devID = 0;
				}
				var packA3 = loginManager.PacketA3.create({
					name: $('#userName').val(),
					pwd: $('#pwd').val(),
					deviceID: devID
				});
				setman.client.tcpConnection.sendPack(new OPacket("A3", true, [0], packA3, loginManager.PacketA3));
				loginManager.unbindButtons();
			}
		});

		$('#pwdResetButton').click(function () {
			if ($('#emailField').hasClass('hidden')) {
				$('#pwdResetButton h2').text('Send Password Reset Email');
				$('#pwdResetButton').blur();
				$('#emailField').removeClass('hidden');
				$('#emailFieldLabel').removeClass('hidden');
				$('#cancelButton').removeClass('hidden');
			}
			else {
				if ($('#emailField').val().length <= 0) {
					loginManager.setErrorMsg('No email entered');
				}
				else {
					var packA4 = loginManager.PacketA4.create({
						email: $('#emailField').val()
					});
					client.tcpConnection.sendPack(new OPacket("A4", true, [0], packA4, loginManager.PacketA4));
					loginManager.unbindButtons();
				}
			}
		});

		$('#cancelButton').click(function () {
			$('#emailField').val('');
			$('#pwdResetButton h2').text('Forgot Password');
			$('#emailField').addClass('hidden');
			$('#emailFieldLabel').addClass('hidden');
			$('#cancelButton').addClass('hidden');
		});
	};

	this.unbindButtons = function () {
		$('#msg').addClass('hidden');
		$('#loginButton').unbind('click');
		$('#pwdResetButton').unbind('click');
		$('#cancelButton').unbind('click');
		$('#loginButton').addClass('processing');
		$('#pwdResetButton').addClass('processing');
		$('#cancelButton').addClass('processing');
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
}
