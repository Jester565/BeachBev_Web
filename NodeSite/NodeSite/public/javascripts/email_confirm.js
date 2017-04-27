'use strict';

var setman = null;
var emailConfirmManager = null;

$('document').ready(function () {
	$("#mBar").load("./mBar.html", function () {
		emailConfirmManager = new EmailConfirmManager();
		setman = new SetupManager(true, emailConfirmManager);
	});
});

function EmailConfirmManager() {
	emailConfirmManager = this;

	this.onProto = function () {
		emailConfirmManager.initPacks();
	}

	this.onOpen = function () {
		emailConfirmManager.sendB2();
	}

	this.onReopen = function () {
		if ($('#checkmark').hasClass('hidden')) {
			emailConfirmManager.sendB2();
		}
	}

	this.initPacks = function () {
		emailConfirmManager.PacketB2 = setman.client.root.lookup("ProtobufPackets.PackB2");
		emailConfirmManager.PacketB3 = setman.client.root.lookup("ProtobufPackets.PackB3");

		setman.client.packetManager.addPKey(new PKey("B3", function (iPack) {
			$('#loading').addClass('hidden');
			var packB3 = emailConfirmManager.PacketB3.decode(iPack.packData);
			if (packB3.success) {
				console.log("Success!");
				$('#checkmark').removeClass('hidden');
				$('#msg').removeClass('hidden');
				$('#msg').addClass('success');
				$('#msg').text(packB3.msg);
			}
			else {
				emailConfirmManager.setErrorMsg(packB3.msg);
			}
		}, emailConfirmManager, "Gets if the confirmation was successful"));
	}

	this.setErrorMsg = function (str) {
		$('#msg').removeClass('hidden');
		$('#msg').text(str);
		$('#msg').focus();
		$('html, body').scrollTo($('#msg'), 100);
	};

	this.sendB2 = function () {
		var url = window.location.href;
		var questionI = url.indexOf('?');
		if (questionI !== -1) {
			emailConfirmManager.emailToken = url.substring(++questionI);
			var packB2 = emailConfirmManager.PacketB2.create({
				emailToken: emailConfirmManager.emailToken
			});
			setman.client.tcpConnection.sendPack(new OPacket("B2", true, [0], packB2, emailConfirmManager.PacketB2));
		} else {
			emailConfirmManager.setErrorMsg("No email token in url");
		}
	}
}
