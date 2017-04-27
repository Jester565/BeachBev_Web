'use strict';

var setman = null;
var emailManager = null;

$('document').ready(function () {
	$("#mBar").load("./mBar.html", function () {
		emailManager = new EmailManager();
		setman = new SetupManager(true, emailManager);
	});
});

function EmailManager() {
	emailManager = this;

	this.onProto = function () {
		emailManager.initPacks();
	}

	this.onOpen = function () {
		var packB4 = emailManager.PacketB4.create({});
		setman.client.tcpConnection.sendPack(new OPacket("B4", true, [0], packB4, emailManager.PacketB4));
	}

	this.onReopen = function() {
		var packB4 = emailManager.PacketB4.create({});
		setman.client.tcpConnection.sendPack(new OPacket("B4", true, [0], packB4, emailManager.PacketB4));
		if (!emailManager.resendButton.isBound()) {
			emailManager.bindButtons();
			setErrorMsg("Request Failed: Connection Lost");
		}
	}

	this.onClose = function () {
		$('#emailDiv').addClass('hidden');
	}

	this.initPacks = function(){
		emailManager.PacketB0 = setman.client.root.lookup("ProtobufPackets.PackB0");
		emailManager.PacketB1 = setman.client.root.lookup("ProtobufPackets.PackB1");
		emailManager.PacketB4 = setman.client.root.lookup("ProtobufPackets.PackB4");
		emailManager.PacketB5 = setman.client.root.lookup("ProtobufPackets.PackB5");
		setman.client.packetManager.addPKey(new PKey("B5", function (iPack) {
			var packB5 = emailManager.PacketB5.decode(iPack.packData);
			emailManager.verifiedEmail = null;
			emailManager.unverifiedEmail = null;
			if (packB5.verifiedEmail !== null && packB5.verifiedEmail.length > 0) {
				emailManager.verifiedEmail = packB5.verifiedEmail;
			}
			if (packB5.unverifiedEmail !== null && packB5.unverifiedEmail.length > 0) {
				emailManager.unverifiedEmail = packB5.unverifiedEmail;
			}
			emailManager.updateEmailDisplay();
		}, emailManager, "Gets the users email information"));

		setman.client.packetManager.addPKey(new PKey("B1", function (iPack) {
			var packB1 = emailManager.PacketB1.decode(iPack.packData);
			if (packB1.success) {
				emailManager.unverifiedEmail = emailManager.requestEmail;
				emailManager.updateEmailDisplay();
			}
			else {
				emailManager.setErrorMsg(packB1.msg);
				emailManager.bindButtons();
			}
		}, emailManager, "Gets the users email information"));
	}

	this.updateEmailDisplay = function () {
		emailManager.bindButtons();
		$('#emailChangeField').addClass('hidden');
		$('#emailChangeFieldLabel').addClass('hidden');
		emailManager.cancelButton.addClass('hidden');
		emailManager.resendButton.addClass('hidden');
		$('#emailChangeField').val('');

		if (emailManager.verifiedEmail === null && emailManager.unverifiedEmail !== null) {
			$('#emailTitle').text('CONFIRM EMAIL');
			$('#emailBody').html('Email: ' + emailManager.unverifiedEmail +
				'<br />Before acccessing your account you must click the\
								link in the email sent to \"' + emailManager.unverifiedEmail + '\"');
			$('#emailDiv').removeClass('hidden');
			emailManager.resendButton.removeClass('hidden');
		}
		else if (emailManager.verifiedEmail !== null && emailManager.unverifiedEmail !== null) {
			$('#emailTitle').text('CONFIRM EMAIL');
			$('#emailBody').html('Current Email: ' + emailManager.verifiedEmail +
				'<br />Unverified Email: ' + emailManager.unverifiedEmail +
				'<br />Your account is associated with the email: \"' + emailManager.verifiedEmail +
				'\". However, you requested to change your email to \"' + emailManager.unverifiedEmail +
				'\". Click the link in the verification email to complete the email change.');
			$('#emailDiv').removeClass('hidden');
			emailManager.resendButton.removeClass('hidden');
		}
		else if (emailManager.verifiedEmail !== null && emailManager.unverifiedEmail === null) {
			$('#emailTitle').text('EMAIL SETTINGS');
			$('#emailBody').text('Email: ' + emailManager.verifiedEmail);
			$('#emailDiv').removeClass('hidden');
		}
		$('#loading').addClass('hidden');
	};

	this.unbindButtons = function () {
		$('#errorMsg').addClass('hidden');
		emailManager.resendButton.unbind();
		emailManager.changeButton.unbind();
		emailManager.cancelButton.unbind();
		emailManager.cancelButton.addClass('processing');
		emailManager.resendButton.addClass('processing');
		emailManager.changeButton.addClass('processing');
	};

	this.bindButtons = function () {
		emailManager.resendButton.removeClass('processing');
		emailManager.changeButton.removeClass('processing');
		emailManager.cancelButton.removeClass('processing');
		emailManager.resendButton.bind();
		emailManager.changeButton.bind();
		emailManager.cancelButton.bind();
	};

	this.setErrorMsg = function (msg) {
		$('#errorMsg').html(msg);
		$('#errorMsg').removeClass('hidden');
		$('#errorMsg').focus();
		$('html, body').scrollTo($('#msg'), 100);
	};

	this.resendButton = new PKeyButton('#resendButton', function () {
		$('errorMsg').addClass('hidden');
		if (emailManager.unverifiedEmail !== null &&
			emailManager.unverifiedEmail.length <= 0) {
			emailManager.setErrorMsg("No unverified email");
		}
		else {
			var packB0 = emailManager.PacketB0.create({
				email: emailManager.unverifiedEmail
			});
			emailManager.requestEmail = emailManager.unverifiedEmail;
			setman.client.tcpConnection.sendPack(new OPacket("B0", true, [0], packB0, emailManager.PacketB0));
			emailManager.unbindButtons();
		}
	});

	this.changeButton = new PKeyButton('#changeButton', function () {
		$('errorMsg').addClass('hidden');
		if ($('#emailChangeField').hasClass('hidden')) {
			$('#emailChangeField').removeClass('hidden');
			$('#emailChangeFieldLabel').removeClass('hidden');
			emailManager.cancelButton.removeClass('hidden');
		}
		else {
			if ($('#emailChangeField').val().length <= 0) {
				emailManager.setErrorMsg("Email not entered");
			}
			else {
				var packB0 = emailManager.PacketB0.create({
					email: $('#emailChangeField').val()
				});
				emailManager.requestEmail = $('#emailChangeField').val();
				setman.client.tcpConnection.sendPack(new OPacket("B0", true, [0], packB0, emailManager.PacketB0));
				emailManager.unbindButtons();
			}
		}
	});

	this.cancelButton = new PKeyButton("#cancelButton", function () {
		$('errorMsg').addClass('hidden');
		$('#emailChangeField').addClass('hidden');
		$('#emailChangeFieldLabel').addClass('hidden');
		emailManager.cancelButton.addClass('hidden');
		$('#emailChangeField').val('');
	});
}
