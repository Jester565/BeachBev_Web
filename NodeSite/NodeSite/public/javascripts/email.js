"use strict";

function EmailManager(root) {
	emailManager = this;
	this.PacketB0 = root.lookup("ProtobufPackets.PackB0");
	this.PacketB1 = root.lookup("ProtobufPackets.PackB1");
	this.PacketB4 = root.lookup("ProtobufPackets.PackB4");
	this.PacketB5 = root.lookup("ProtobufPackets.PackB5");
	client.packetManager.addPKey(new PKey("B5", function (iPack) {
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
	}, this, "Gets the users email information"));

	client.packetManager.addPKey(new PKey("B1", function (iPack) {
		var packB1 = emailManager.PacketB1.decode(iPack.packData);
		if (packB1.success) {
			emailManager.unverifiedEmail = emailManager.requestEmail;
			emailManager.updateEmailDisplay();
		}
		else {
			emailManager.setErrorMsg(packB1.msg);
			emailManager.bindButtons();
		}
	}, this, "Gets the users email information"));

	this.updateEmailDisplay = function () {
		emailManager.bindButtons();
		$('#emailChangeField').addClass('hidden');
		$('#emailChangeFieldLabel').addClass('hidden');
		$('#cancelButton').addClass('hidden');
		$('#resendButton').addClass('hidden');
		$('#emailChangeField').val('');

		if (emailManager.verifiedEmail === null && emailManager.unverifiedEmail !== null) {
			$('#emailTitle').text('CONFIRM EMAIL');
			$('#emailBody').html('Email: ' + emailManager.unverifiedEmail +
				'<br />Before acccessing your account you must click the\
								link in the email sent to \"' + emailManager.unverifiedEmail + '\"');
			$('#emailDiv').removeClass('hidden');
			$('#resendButton').removeClass('hidden');
		}
		else if (emailManager.verifiedEmail !== null && emailManager.unverifiedEmail !== null) {
			$('#emailTitle').text('CONFIRM EMAIL');
			$('#emailBody').html('Current Email: ' + emailManager.verifiedEmail +
				'<br />Unverified Email: ' + emailManager.unverifiedEmail +
				'<br />Your account is associated with the email: \"' + emailManager.verifiedEmail +
				'\". However, you requested to change your email to \"' + emailManager.unverifiedEmail +
				'\". Click the link in the verification email to complete the email change.');
			$('#emailDiv').removeClass('hidden');
			$('#resendButton').removeClass('hidden');
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
		$('#resendButton').unbind('click');
		$('#changeButton').unbind('click');
		$('#cancelButton').unbind('click');
		$('#resendButton').addClass('processing');
		$('#changeButton').addClass('processing');
		$('#resendButton').addClass('processing');
	};

	this.bindButtons = function () {
		$('#resendButton').removeClass('processing');
		$('#changeButton').removeClass('processing');
		$('#resendButton').removeClass('processing');

		$('#resendButton').click(function () {
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
				client.tcpConnection.sendPack(new OPacket("B0", true, [0], packB0, emailManager.PacketB0));
				$('#resendButton').unbind('click');
				emailManager.unbindButtons();
			}
		});

		$('#changeButton').click(function () {
			$('errorMsg').addClass('hidden');
			if ($('#emailChangeField').hasClass('hidden')) {
				$('#emailChangeField').removeClass('hidden');
				$('#emailChangeFieldLabel').removeClass('hidden');
				$('#cancelButton').removeClass('hidden');
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
					client.tcpConnection.sendPack(new OPacket("B0", true, [0], packB0, emailManager.PacketB0));
					emailManager.unbindButtons();
				}
			}
		});

		$('#cancelButton').click(function () {
			$('errorMsg').addClass('hidden');
			$('#emailChangeField').addClass('hidden');
			$('#emailChangeFieldLabel').addClass('hidden');
			$('#cancelButton').addClass('hidden');
			$('#emailChangeField').val('');
		});
	};

	this.setErrorMsg = function (msg) {
		$('#errorMsg').html(msg);
		$('#errorMsg').removeClass('hidden');
		$('html, body').animate({
			scrollTop: $("#errorMsg").offset().top
		}, 100);
	};
	var packB4 = emailManager.PacketB4.create({
	});
	client.tcpConnection.sendPack(new OPacket("B4", true, [0], packB4, emailManager.PacketB4));
}

var emailManager;
var innerLoginManager;

var client = new Client(function (root) {
	console.log("ON LOAD CALLED");
	innerLoginManager = new InnerLoginManager(client, root,
		function () {
			console.log("LOGGED IN");
			emailManager = new EmailManager(client.root);
		});
	client.tcpConnection.onclose = function () {
		redirect('./noServer.html');
	};
});