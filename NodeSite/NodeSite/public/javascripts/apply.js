'use strict';

var setman = null;
var applyManager = null;

$('document').ready(function () {
	$("#mBar").load("./mBar.html", function () {
		applyManager = new ApplyManager();
		setman = new SetupManager(false, applyManager);
	});
});

function ApplyManager() {
	applyManager = this;

	this.onProto = function () {
		applyManager.initPacks();
	}

	this.onOpen = function () {
		applyManager.initDisplay();
	}

	this.onReopen = function () {
		if (applyManager.applyButton.bind()) {
			applyManager.setErrorMsg("Request Failed: Connection Lost");
		}
	}

	this.initDisplay = function () {
		$('#loading').addClass('hidden');
		$('#workerImg').removeClass('hidden');
		$('#pitch').removeClass('hidden');
		$('#applyDiv').removeClass('hidden');
	}

	this.initPacks = function () {
		this.PacketA0 = setman.client.root.lookup("ProtobufPackets.PackA0");
		this.PacketA1 = setman.client.root.lookup("ProtobufPackets.PackA1");

		setman.client.packetManager.addPKey(new PKey("A1", function (iPack) {
			var packA1 = applyManager.PacketA1.decode(iPack.packData);
			if (packA1.pwdToken === null || packA1.pwdToken.length <= 0) {
				applyManager.applyButton.bind();
				applyManager.setErrorMsg(packA1.msg);
			}
			else {
				Cookies.set('pwdToken', packA1.pwdToken, { expires: 1, path: '/', domain: window.location.hostname, secure: true });
				Cookies.set('deviceID', packA1.deviceID, { path: '/', domain: window.location.hostname, secure: true });
				Cookies.set('eID', packA1.eID, { path: '/', domain: window.location.hostname, secure: true });
				Redirect('./employee.html');
			}
		}, this, "Gets the success of the login"));
	}

	this.submit = function () {
		if ($('#userName').val().length === 0) {
			applyManager.setErrorMsg("No username was entered");
		}
		else if ($('#pwd').val().length < 8) {
			applyManager.setErrorMsg("Password must be longer than 8 characters");
		}
		else if ($('#pwd').val() !== $('#pwdConfirm').val()) {
			applyManager.setErrorMsg("Confirm password does not match password");
		}
		else if (!String($('#email').val()).includes('@')) {
			applyManager.setErrorMsg("Invalid email");
		}
		else {
			var packA0 = applyManager.PacketA0.create({ name: $('#userName').val(), pwd: $('#pwd').val(), email: $('#email').val() });
			setman.client.tcpConnection.sendPack(new OPacket("A0", true, [0], packA0, applyManager.PacketA0));
			$('#msg').addClass('invisible');
			applyManager.applyButton.removeClass('error');
			applyManager.applyButton.addClass('load');
			applyManager.applyButton.text('PROCESSING', 'h2');
			applyManager.applyButton.unbind();
		}
	};

	this.setErrorMsg = function(str) {
		$('#msg').removeClass('invisible');
		$('#msg').text(str);
		$('#msg').focus();
		$('html, body').scrollTo($('#msg'), 100);
		applyManager.applyButton.addClass('error');
		applyManager.applyButton.removeClass('load');
		applyManager.applyButton.text('ERROR', 'h2');
	}

	this.applyButton = new PKeyButton('#applyButton', applyManager.submit);
}
