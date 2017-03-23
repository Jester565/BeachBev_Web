"use strict";

function setErrorMsg(str) {
	$('#msg').removeClass('invisible');
	$('#msg').text(str);
	$('#msg').focus();
	this.scrollTo('max',100);
	$('#applyButton').addClass('error');
	$('#applyButton').removeClass('load');
	$('#applyButton h2').text('ERROR');
}

$('#userName, #pwd, #pwdConfirm, #email').focus(function () {
	if ($('#applyButton').hasClass('error')) {
		$('#applyButton').removeClass('error');
		$('#msg').addClass('invisible');
		$('#applyButton h2').text('APPLY NOW');
	}
});

function ApplyManager(root) {
	$('#loading').addClass('hidden');
	$('#workerImg').removeClass('hidden');
	$('#pitch').removeClass('hidden');
	$('#applyDiv').removeClass('hidden');
	this.PacketA0 = root.lookup("ProtobufPackets.PackA0");
	this.PacketA1 = root.lookup("ProtobufPackets.PackA1");

	client.packetManager.addPKey(new PKey("A1", function (iPack) {
		var packA1 = applyManager.PacketA1.decode(iPack.packData);
		if (packA1.pwdToken === null || packA1.pwdToken.length <= 0) {
			$('#applyButton').click(applyManager.submit);
			setErrorMsg(packA1.msg);
		}
		else {
			Cookies.set('pwdToken', packA1.pwdToken, { expires: 1, path: '/', domain: 'beachbevs.com', secure: true });
			Cookies.set('deviceID', packA1.deviceID, { path: '/', domain: 'beachbevs.com', secure: true });
			Cookies.set('eID', packA1.eID, { path: '/', domain: 'beachbevs.com', secure: true });
			redirect('./email.html');
		}
	}, this, "Gets the success of the login"));

	this.submit = function () {
		console.log("submit called");
		if ($('#userName').val().length === 0) {
			setErrorMsg("No username was entered");
		}
		else if ($('#pwd').val().length < 8) {
			setErrorMsg("Password must be longer than 8 characters");
		}
		else if ($('#pwd').val() !== $('#pwdConfirm').val()) {
			setErrorMsg("Confirm password does not match password");
		}
		else if (!String($('#email').val()).includes('@')) {
			setErrorMsg("Invalid email");
		}
		else {
			var packA0 = applyManager.PacketA0.create({ name: $('#userName').val(), pwd: $('#pwd').val(), email: $('#email').val() });
			client.tcpConnection.sendPack(new OPacket("A0", true, [0], packA0, applyManager.PacketA0));
			$('#msg').addClass('invisible');
			$('#applyButton').removeClass('error');
			$('#applyButton').addClass('load');
			$('#applyButton h2').text('PROCESSING');
			$('#applyButton').unbind('click');
		}
	};
	$('#applyButton').click(this.submit);
}

var applyManager;

var client = new Client(function (root) {
	console.log("ON LOAD CALLED");
	client.tcpConnection.onopen = function () {
		applyManager = new ApplyManager(client.root);
	};
	client.tcpConnection.onclose = function () {
		redirect('./noServer.html');
	};
});