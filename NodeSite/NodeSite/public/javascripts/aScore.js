function AvailibleManager() {
	this.initPacks = function () {
		client.builder.define("AvailiblePacks");
		client.builder.create([
			{
				"name": "PackA3",
				"fields": [
					{
						"rule": "optional",
						"type": "bool",
						"name": "confirm",
						"id": 1
					}
				]
			},
			{
				"name": "PackG0",
				"fields": [
					{
						"rule": "optional",
						"type": "double",
						"name": "latitude",
						"id": 1
					},
					{
						"rule": "optional",
						"type": "double",
						"name": "longitude",
						"id": 2
					}
				]
			},
			{
				"name": "PackG1",
				"fields": [
					{
						"rule": "optional",
						"type": "bool",
						"name": "success",
						"id": 1
					},
					{
						"rule": "repeated",
						"type": "string",
						"name": "itemNames",
						"id": 2
					},
					{
						"rule": "repeated",
						"type": "uint32",
						"name": "itemAmounts",
						"id": 3
					},
					{
						"rule": "optional",
						"type": "float",
						"name": "totalCost",
						"id": 4
					},
					{
						"rule": "optional",
						"type": "double",
						"name": "latitude",
						"id": 5
					},
					{
						"rule": "optional",
						"type": "double",
						"name": "longitude",
						"id": 6
					},
					{
						"rule": "optional",
						"type": "uint32",
						"name": "orderID",
						"id": 7
					},
					{
						"rule": "optional",
						"type": "int32",
						"name": "orderTime",
						"id": 8
					}
				]
			},
			{
				"name": "PackG2",
				"fields": [
					{
						"rule": "optional",
						"type": "uint32",
						"name": "orderID",
						"id": 1
					},
					{
						"rule": "optional",
						"type": "uint32",
						"name": "oState",
						"id": 2
					}
				]
			}
		]);
		client.builder.reset();
		var packSetup = client.builder.build("AvailiblePacks");
		this.PackA3 = packSetup.PackA3;
		this.PackG0 = packSetup.PackG0;
		this.PackG1 = packSetup.PackG1;
		this.PackG2 = packSetup.PackG2;
	};
	this.initPacks();
}

client.packetManager.addPKey(new PKey("A3", function (iPack) {
	console.log("Key A3 RECEIVED");
	if ($('#order').hasClass('hidden')) {
		var packG0 = new availibleManager.PackG0(33.861206, -118.405477);
		client.tcpConnection.sendPack(new OPacket("G0", true, [0], packG0));
		console.log("G0 Sent");
	}
}, this, "Gets the success of the login"));

client.packetManager.addPKey(new PKey("G1", function (iPack) {
	var packG1 = availibleManager.PackG1.decode(iPack.packData);
	console.log("G1 Recieved");
	if (packG1.success) {
		$('#oTime').html(new String(((new Date().getTime() / 1000 - packG1.orderTime) / 60).toFixed(0)) + " mins");
		var itemStr = new String();
		for (let i = 0; i < packG1.itemNames.length; i++) {
			itemStr += new String(packG1.itemNames[i]);
			itemStr += ": ";
			itemStr += new String(packG1.itemAmounts[i]);
			itemStr += "<br>";
		}
		$('#oItems').html(itemStr);
		$('#oCost').html(new String((packG1.totalCost).toFixed(2)));
		$('#order').removeClass('hidden');
		orderManager.oID = packG1.orderID;
		orderManager.oLatitude = packG1.latitude;
		orderManager.oLongitude = packG1.longitude;
	}
}, this, "Gets the success of the login"));

var availibleManager = new AvailibleManager();