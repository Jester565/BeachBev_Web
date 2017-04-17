'use strict';

var innerLoginManager = null;
var employeeManager = null;
var INVALID_ASTATE = -2;
var UNVERIFIED_ASTATE = -1;
var UNACCEPTED_ASTATE = 0;
var ACCEPTED_ASTATE = 1;
var DECLINED_ASTATE = 2;
var EMPLOYEE_ASTATE = 3;

var client = new Client(function (root) {
	innerLoginManager = new InnerLoginManager(client, root,
		function () {
			if (employeeManager === null) {
				employeeManager = new EmployeeManager(client.root);
			}
			else {
				if ($('#acceptOfferButton').hasClass('processing')) {
					employeeManager.bindOfferButtons();
					employeeManager.setErrorMsg(packE7.msg);
				}
				if ($('#linkDiv').hasClass('hidden')) {
					employeeManager.sendE4();
				}
				HandleConnectServer();
			}
		});
	client.tcpConnection.onclose = function () {
		HandleNoServer();
	}
});

function EmployeeManager(root) {
	employeeManager = this;

	this.setErrorMsg = function (msg) {
		$('#msg').text(msg);
		$('#msg').removeClass('hidden');
		$('#msg').focus();
		$('html, body').scrollTo($('#msg'), 100);
	}

	this.clearErrorMsg = function () {
		$('#msg').addClass('hidden');
	}

	this.initDisplay = function () {
		$('#linkDiv').removeClass('hidden');
		$('#loading').addClass('hidden');
	}

	this.setUnverifiedDisplay = function () {
		$('#summary').html("Verify the email sent to you by BeachBevs before accessing the rest or your account.\
			<a href= \"email.html\" > Click here</a > to change your email or resend the verification message.");
		$('#emailStep').addClass('active');
		$('#resumeLink').addClass('hidden');
		$('.container').removeClass('hidden');
	}

	this.setResumeDisplay = function () {
		$('#summary').html("<a href=\"resume.html\">Click here</a> to upload a pdf of your resume");
		$('#resumeStep').addClass('active');
		$('#emailStep').addClass('passed');
		$('.container').removeClass('hidden');
	}

	this.setWaitDisplay = function () {
		$('#summary').html("We will make a decision by one week. Thank you for applying!");
		$('#waitStep').addClass('active');
		$('#resumeStep').addClass('passed');
		$('#emailStep').addClass('passed');
		$('.container').removeClass('hidden');
	}

	this.setOfferDisplay = function () {
		$('#offerDiv').removeClass('hidden');
	}

	this.bindOfferButtons = function () {
		$('#acceptOfferButton').removeClass('processing');
		$('#acceptOfferButton').click(function () {
			employeeManager.sendOffer(true);
		});
		$('#declineOfferButton').removeClass('processing');
		$('#declineOfferButton').click(function () {
			employeeManager.sendOffer(false);
		});
	}

	this.sendOffer = function (accept) {
		$('#acceptOfferButton').unbind('click');
		$('declineOfferButton').unbind('click');
		$('#acceptOfferButton').addClass('processing');
		$('#declineOfferButton').addClass('processing');
		employeeManager.sendE6(accept);
	}

	this.initPackets = function (root) {
		employeeManager.PacketE4 = root.lookup("ProtobufPackets.PackE4");
		employeeManager.PacketE5 = root.lookup("ProtobufPackets.PackE5");
		employeeManager.PacketE6 = root.lookup("ProtobufPackets.PackE6");
		employeeManager.PacketE7 = root.lookup("ProtobufPackets.PackE7");
		employeeManager.PacketD3 = root.lookup("ProtobufPackets.PackD3");
		employeeManager.PacketD4 = root.lookup("ProtobufPackets.PackD4");
		client.packetManager.addPKey(new PKey("E5", function (iPack) {
			var packE5 = employeeManager.PacketE5.decode(iPack.packData);
			if (packE5.aState <= INVALID_ASTATE) {
				employeeManager.setErrorMsg(packE5.msg);
				employeeManager.initDisplay();
			}
			else if (packE5.aState <= UNVERIFIED_ASTATE) {
				employeeManager.setUnverifiedDisplay();
				employeeManager.initDisplay();
			}
			else if (packE5.aState <= UNACCEPTED_ASTATE) {
				employeeManager.sendD3();
			}
			else if (packE5.aState <= ACCEPTED_ASTATE) {
				employeeManager.bindOfferButtons();
				employeeManager.setOfferDisplay();
				employeeManager.initDisplay();
			}
		}));

		client.packetManager.addPKey(new PKey("D4", function (iPack) {
			var packD4 = employeeManager.PacketD4.decode(iPack.packData);
			if (!packD4.hasResume) {
				employeeManager.setResumeDisplay();
			}
			else
			{
				employeeManager.setWaitDisplay();
			}
			employeeManager.initDisplay();
		}));

		client.packetManager.addPKey(new PKey("E7", function (iPack) {
			var packE7 = employeeManager.PacketE7.decode(iPack.packData);
			if (packE7.success) {
				//handle added employee
			}
			else
			{
				employeeManager.bindOfferButtons();
				setErrorMsg(packE7.msg);
			}
		}));

		employeeManager.sendE4();
	}

	this.sendD3 = function () {
		employeeManager.clearErrorMsg();
		var packD3 = employeeManager.PacketD3.create({});
		client.tcpConnection.sendPack(new OPacket("D3", true, [0], packD3, employeeManager.PacketD3));
	}

	this.sendE4 = function () {
		employeeManager.clearErrorMsg();
		var packE4 = employeeManager.PacketE4.create({});
		client.tcpConnection.sendPack(new OPacket("E4", true, [0], packE4, employeeManager.PacketE4));
	}

	this.sendE6 = function (nAccept) {
		employeeManager.clearErrorMsg();
		var packE6 = employeeManager.PacketE6.create({accept: nAccept});
		client.tcpConnection.sendPack(new OPacket("E6", true, [0], packE6, employeeManager.PacketE6));
	}

	this.initPackets(root);
}
