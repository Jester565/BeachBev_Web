'use strict';

var mbarManager = null;

function MBarManager() {
	mbarManager = this;

	var dotInterval = null;

	this.initPacks = function () {
		mbarManager.PacketC2 = setman.client.root.lookup("ProtobufPackets.PackC2");
		mbarManager.PacketC3 = setman.client.root.lookup("ProtobufPackets.PackC3");

		setman.client.packetManager.addPKey(new PKey("C3"), function (iPack) {
			var packC3 = mbarManager.PacketC3.decode(iPack.packData);
			if (packC3.name === null) {
				console.error("Failed to aquire name (C3 handler)");
			}
			else
			{
				mbarManager._initSubEmpDivDisplay(packC3.name);
			}
		});
	}

	this.initSubEmpDiv = function () {
		var packC2 = mbarManager.PacketC2.create({ });
		setman.client.tcpConnection.sendPack(new OPacket("C2", true, [0], packC2, mbarManager.PacketC2));
	}

	this._initSubEmpDivDisplay = function (name) {
		$('#subEmpName').text(name);

		$('#subEmpLogOut').click(function () {
			Cookies.remove('eID');
			Cookies.remove('deviceID');
			Cookies.remove('pwdToken');
			Redirect("./login.html");
		});

		$('#subEmpHome').click(function () {
			Redirect("./employee.html");
		});

		$('#subEmpEmail').click(function () {
			Redirect("./email.html");
		});

		$('#subEmpResume').click(function () {
			Redirect("./resume.html");
		});

		$('#subEmpMaster').click(function () {
			Redirect("./master.html");
		});

		$('#accountDiv').removeClass('hidden');
	}

	this.initHandlers = function () {
		$('#sideMenuButton').click(function () {
			$(this).toggleClass('open');
			$('#sideMenuNav').toggleClass('open');
			$('#headerRectBorderCover').toggleClass('open');
		});

		$('#dropArrowImg').click(function () {
			if ($('#employeeMenuDiv').hasClass('hidden')) {
				$('#employeeMenuDiv').removeClass('hidden');
				$('#dropArrowImg').addClass('dropped');
			}
			else {
				$('#employeeMenuDiv').addClass('hidden');
				$('#dropArrowImg').removeClass('dropped');
			}
		});

		$('#homeLink').click(function () {
			Redirect("./index.html");
		});

		$('#employeeMenuTitle').click(function () {
			Redirect("./employee.html");
		});

		$('#loginLink').click(function () {
			Redirect("./login.html");
		});

		$('#applyLink').click(function () {
			Redirect("./apply.html");
		});
	}

	this.showNoServer = function() {
		$('#noServerDiv').removeClass('hidden');
		if (dotInterval === null) {
			dotInterval = setInterval(function () {
				var dotText = $('#noServerDiv > p > a').text();
				if (dotText.length < 3) {
					dotText += ".";
				}
				else {
					dotText = "";
				}
				$('#noServerDiv > p > a').text(dotText);
			}, 1000);
		}
	}

	this.hideNoServer = function() {
		$('#noServerDiv').addClass('hidden');
		if (dotInterval !== null) {
			clearInterval(dotInterval);
			dotInterval = null;
		}
	}

	this.initHandlers();
}
