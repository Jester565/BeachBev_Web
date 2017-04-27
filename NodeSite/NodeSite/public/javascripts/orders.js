function OrderManager() {
	this.initPacks = function () {
		client.builder.define("LoginPacks");
		client.builder.create([
			{
				"name": "PackF0",
				"fields": [
					{
						"rule": "optional",
						"type": "string",
						"name": "username",
						"id": 1
					},
					{
						"rule": "optional",
						"type": "string",
						"name": "token",
						"id": 2
					}
				]
			},
			{
				"name": "PackF1",
				"fields": [
					{
						"rule": "optional",
						"type": "bool",
						"name": "success",
						"id": 1
					}
				]
			}
		]);
		client.builder.reset();
		var packSetup = client.builder.build("LoginPacks");
		this.PackF0 = packSetup.PackF0;
		this.PackF1 = packSetup.PackF1;
	};
	this.initPacks();
	client.packetManager.addPKey(new PKey("F1", function (iPack) {
		var packF1 = orderManager.PackF1.decode(iPack.packData);
		if (packF1.success) {
			var packG0 = new availibleManager.PackG0(33.861206, -118.405477);
			client.tcpConnection.sendPack(new OPacket("G0", true, [0], packG0));
			var name = localStorage.getItem("name");
			$('#name').html(name);
		}
		else {
			if (window.confirm("Your Login Token Has Expired... Click YES To Redirect")) {
				Redirect("login.html");
			}
		}
	}, this, "Gets the success of the login"));
	$('#settingButton').click(function () {
		$('#settings').toggleClass("down");
	});
	$('#logout').click(function () {
		localStorage.removeItem("token");
		localStorage.removeItem("name");
		Redirect("login.html");
	});
	this.oLongitude = 0;
	this.oLatitude = 0;
	this.oID = 0;
	$('#oNavigate').click(function () {
		var gMapStr = "http://maps.google.com/maps?&z=17&mrt=yp&t=k&q=";
		console.log(orderManager.oLatitude);
		console.log(orderManager.oLongitude);
		gMapStr += new String((orderManager.oLatitude).toFixed(8));
		gMapStr += "+";
		gMapStr += new String((orderManager.oLongitude).toFixed(8));
		console.log(gMapStr);
		Redirect(gMapStr);
	});
	$('#oDone').click(function () {
		var packG2 = new availibleManager.PackG2();
		packG2.orderID = orderManager.oID;
		packG2.oState = 1;
		client.tcpConnection.sendPack(new OPacket("G2", true, [0], packG2));
		var packG0 = new availibleManager.PackG0(true);
		client.tcpConnection.sendPack(new OPacket("G0", true, [0], packG0));
		$('#order').addClass('hidden');
	});
	$('#oFail').click(function () {
		var packG2 = new availibleManager.PackG2();
		packG2.orderID = orderManager.oID;
		packG2.oState = 2;
		client.tcpConnection.sendPack(new OPacket("G2", true, [0], packG2));
		var packG0 = new availibleManager.PackG0(true);
		client.tcpConnection.sendPack(new OPacket("G0", true, [0], packG0));
		$('#order').addClass('hidden');
	});
}

var client = new Client("localhost", "24560");
client.tcpConnection.onopen = function () {
	var name = localStorage.getItem("name");
	var token = localStorage.getItem("token");
	if (name !== null && token !== null) {
		var packF0 = new orderManager.PackF0(name, token);
		client.tcpConnection.sendPack(new OPacket("F0", true, [0], packF0));
	}
	else {
		if (window.confirm("You Don't Have A Login Token... Click YES To Redirect")) {
			Redirect("login.html");
		}
	}
};

client.tcpConnection.onclose = function () {
	alert("The Server Is Unavailible...");
};

var orderManager = new OrderManager();