'use strict';

var innerLoginManager;
var employeeManager;
var INVALID_ASTATE = -2;
var UNVERIFIED_ASTATE = -1;
var UNACCEPTED_ASTATE = 0;

var client = new Client(function (root) {
	innerLoginManager = new InnerLoginManager(client, root,
		function () {
			employeeManager = new EmployeeManager(client.root);
		});
	client.tcpConnection.onclose = function () {
		redirect('./noServer.html');
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
		$('.container').removeClass('hidden');
		$('#linkDiv').removeClass('hidden');
		$('#loading').addClass('hidden');
	}

	this.setUnverifiedDisplay = function () {
		$('#summary').html("Verify the email sent to you by BeachBevs before accessing the rest or your account.\
			<a href= \"email.html\" > Click here</a > to change your email or resend the verification message.");
		$('#emailStep').addClass('active');
		$('#resumeLink').addClass('hidden');
	}

	this.setResumeDisplay = function () {
		$('#summary').html("<a href=\"resume.html\">Click here</a> to upload a pdf of your resume");
		$('#resumeStep').addClass('active');
		$('#emailStep').addClass('passed');
	}

	this.setWaitDisplay = function () {
		$('#summary').html("We will make a decision by one week. Thank you for applying!");
		$('#waitStep').addClass('active');
		$('#resumeStep').addClass('passed');
		$('#emailStep').addClass('passed');
	}

	this.initPackets = function (root) {
		employeeManager.PacketE4 = root.lookup("ProtobufPackets.PackE4");
		employeeManager.PacketE5 = root.lookup("ProtobufPackets.PackE5");
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
			else {
				
			}
		}));

		client.packetManager.addPKey(new PKey("D4", function (iPack) {
			var packD4 = emloyeeManager.PacketD4.decode(iPack.packData);
			if (packD4.hasResume) {
				employeeManager.setResumeDisplay();
			}
			else
			{
				employeeManager.setWaitDisplay();
			}
			employeeManager.initDisplay();
		}));

		employeeManager.sendE4();
	}

	this.sendD3 = function () {
		var packD3 = employeeManager.PacketD3.create({});
		client.tcpConnection.sendPack(new OPacket("D3", true, [0], packD3, employeeManager.PacketD3));
	}

	this.sendE4 = function () {
		var packE4 = employeeManager.PacketE4.create({});
		client.tcpConnection.sendPack(new OPacket("E4", true, [0], packE4, employeeManager.PacketE4));
	}

	this.initPackets(root);
}
