'use strict';

function MenuItem(id, name, price) {
	this.amount = 0;
	this.id = id;
	this.name = name;
	this.price = price;
	this.tabY = 0;
	this.showSubtract = false;
	this.showAdd = false;
	this.tabDown = false;
	this.tabUp = false;

	this.getHeight = function (w) {
		return 360 + this.tabY;
	};

	this.draw = function (c, ctx, x, y, w, rate) {
		if (this.tabDown) {
			this.tabY += 13.0 * rate;
			if (this.tabY > 220) {
				this.tabY = 220;
				this.tabDown = false;
			}
		}
		if (this.tabUp) {
			this.tabY -= 13.0 * rate;
			if (this.tabY < 0) {
				this.tabY = 0;
				this.tabUp = false;
			}
		}
		var subtractImg = null;
		if (this.tabY > 0) {
			var tabImg = document.getElementById('itemTab');
			ctx.drawImage(tabImg, x + 297, y + 25 + this.tabY, 768, 224);
			ctx.globalAlpha = this.tabY / 220;
			var img = document.getElementById('itemBackSelected');
			ctx.drawImage(img, x, y, w, 330);
			ctx.font = "60px Bernard";
			ctx.textAlign = "center";
			ctx.fillText("$" + this.price.toFixed(2), x + 510, y + 190 + this.tabY);
			ctx.textAlign = "center";
			ctx.fillText("$" + (this.price.toFixed(2) * this.amount).toFixed(2), x + 860, y + 190 + this.tabY);
			ctx.font = "100px Bernard";
			ctx.fillText(this.amount, x + w / 2, y + 155 + this.tabY);
			if (this.showAdd) {
				var addImg = document.getElementById('itemAdd');
				ctx.drawImage(addImg, x + 938, y, 424, 330);
			}
			if (this.showSubtract) {
				subtractImg = document.getElementById('itemSubtract');
				ctx.drawImage(subtractImg, x, y, 418, 330);
			}
			ctx.globalAlpha = 1;
		}
		if (this.tabY < 220) {
			ctx.globalAlpha = 1 - this.tabY / 220;
			img = document.getElementById('itemBack');
			ctx.drawImage(img, x, y, w, 330);
			ctx.font = "120px Bernard";
			ctx.textAlign = "center";
			ctx.fillText("$" + this.price.toFixed(2), x + 200, y + 170);
			if (this.showAdd) {
				addImg = document.getElementById('itemAdd');
				ctx.drawImage(addImg, x + 938, y, 424, 330);
			}
			if (this.showSubtract) {
				subtractImg = document.getElementById('itemSubtract');
				ctx.drawImage(subtractImg, x, y, 418, 330);
			}
			ctx.globalAlpha = 1;
		}
		ctx.font = "120px Bernard";
		ctx.textAlign = "center";
		ctx.fillText(this.name, w / 2, y + 170);
	};
	this.checkClick = function (x, y, w, mx, my) {
		console.log("Check Click");
		if (this.amount === 0) {
			if (mx < w && mx > w - 394 && my > y && my < y + 254) {
				this.amount = 1;
				this.showAdd = true;
				this.tabUp = false;
				this.tabDown = true;
				setTimeout(function (item) {
					item.showAdd = false;
				}, 80, this);
				return true;
			}
		}
		else {
			if (mx < w && mx > w - 394 && my > y && my < y + 254) {
				this.amount++;
				this.showAdd = true;
				setTimeout(function (item) {
					item.showAdd = false;
				}, 80, this);
				return true;
			}
			if (mx < 400 && mx > 0 && my > y && my < y + 254) {
				this.amount--;
				this.showSubtract = true;
				if (this.amount === 0) {
					this.tabDown = false;
					this.tabUp = true;
				}
				setTimeout(function (item) {
					item.showSubtract = false;
				}, 80, this);
				return true;
			}
		}
		return false;
	};
}

function ItemManager(menuItems) {
	this.c = document.getElementById('itemCanvas');
	this.reviewMode = false;
	this.rectCoverW = 0;
	$('#reviewOrder').click(function (evt) {
		itemManager.reviewMode = true;
		var table = document.getElementById("reviewTable");
		var tableRowI = 1;
		var totalCost = 0;
		for (let item of itemManager.menuItems) {
			if (item.amount > 0) {
				var row = table.insertRow(tableRowI);
				var nameCell = row.insertCell(0);
				var priceCell = row.insertCell(1);
				var amountCell = row.insertCell(2);
				var totalCostCell = row.insertCell(3);
				nameCell.innerHTML = item.name;
				priceCell.innerHTML = "$" + item.price.toFixed(2);
				amountCell.innerHTML = item.amount;
				totalCostCell.innerHTML = "$" + (item.price * item.amount).toFixed(2);
				totalCost += item.price * item.amount;
				tableRowI++;
			}
		}
		$('#reviewOrder').hide();
		if (tableRowI > 1) {
			$('#reviewTable').fadeIn();
		}
		else {
			$('#noOrder').fadeIn();
		}
		$('#totalCost').html("Total: $" + totalCost.toFixed(2));
	});
	$('#editOrder').click(function (evt) {
		$('#noOrder').hide();
		$('#editOrder').hide();
		$('#reviewTable').hide();
		$('#confirmOrder').hide();
		$('#orderName').hide();
		$('#totalCost').hide();
		for (let i = document.getElementById("reviewTable").rows.length - 1; i > 0; i--) {
			document.getElementById("reviewTable").deleteRow(i);
		}
		$('#reviewOrder').fadeIn();
		itemManager.reviewMode = false;
	});
	$('#confirmOrder').click(function (evt) {
		if ($('#orderNameField').val().length === 0) {
			alert("Please Enter Your Name");
		}
		else {
			if (navigator.geolocation.getCurrentPosition(sendA0, handleError)) {
			}
			else {
			}
		}
	});
	$('#itemCanvas').click(function (evt) {
		var rect = itemManager.c.getBoundingClientRect();
		var mx = (evt.clientX - rect.left) / (rect.right - rect.left) * itemManager.c.width;
		var my = (evt.clientY - rect.top) / (rect.bottom - rect.top) * itemManager.c.height;
		var y = 0;
		if (!itemManager.reviewMode) {
			for (let item of itemManager.menuItems) {
				if (item.checkClick(0, y, itemManager.c.width, mx, my)) {
					break;
				}
				y += item.getHeight(itemManager.c.width);
			}
		}
	});
	this.ctx = this.c.getContext("2d");
	this.menuItems = menuItems;
	this.refresh = function () {
		var rate = (Date.now() - itemManager.lastTime) / 16.66666;
		itemManager.lastTime = Date.now();
		itemManager.c.width = 1362;
		var canvasH = 0;
		if (!itemManager.reviewMode) {
			for (let item of itemManager.menuItems) {
				canvasH += item.getHeight(itemManager.c.width);
			}
			itemManager.c.height = canvasH;
		}
		if (itemManager.rectCoverW < itemManager.c.width / 2 + 1) {
			var y = 0;
			for (let item of itemManager.menuItems) {
				item.draw(itemManager.c, itemManager.ctx, 0, y, itemManager.c.width, rate);
				y += item.getHeight(itemManager.c.width);
			}
		}
		if (itemManager.reviewMode) {
			if (itemManager.rectCoverW < itemManager.c.width / 2) {
				itemManager.rectCoverW += 37 * rate;
			}
			else {
				itemManager.rectCoverW = itemManager.c.width / 2 + 1;
			}
		}
		else {
			if (itemManager.rectCoverW > 0) {
				itemManager.rectCoverW -= 37 * rate;
			}
			else {
				itemManager.rectCoverW = 0;
			}
		}
		if (itemManager.rectCoverW > 0) {
			itemManager.ctx.fillStyle = "#FFFFFF";
			itemManager.ctx.fillRect(0, 0, itemManager.rectCoverW, itemManager.c.height);
			itemManager.ctx.fillRect(itemManager.c.width - itemManager.rectCoverW, 0, itemManager.rectCoverW, itemManager.c.height);
		}
		if (itemManager.rectCoverW === itemManager.c.width / 2 + 1) {
			if (itemManager.c.height - 60 * rate < 0) {
				itemManager.c.height = 0;
				if (document.getElementById("reviewTable").rows.length > 1) {
					$('#confirmOrder').fadeIn();
					$('#totalCost').fadeIn();
					$('#orderName').fadeIn();
				}
				$('#editOrder').fadeIn();
			}
			else if (itemManager.c.height > 0) {
				itemManager.c.height -= 60 * rate;
				if (itemManager.c.height === 0) {
					itemManager.c.height = -1;
				}
			}
		}
		requestAnimationFrame(itemManager.refresh);
	};
	this.lastTime = Date.now();
	requestAnimationFrame(this.refresh);
}

var itemManager;

function MenuManager() {
	this.initPacks = function () {
		client.builder.define("MenuPacks");
		client.builder.create([
			{
				"name": "PackH1",
				"fields": [
					{
						"rule": "optional",
						"type": "uint32",
						"name": "state",
						"id": 1
					}
				]
			},
			{
				"name": "PackA0",
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
					},
					{
						"rule": "repeated",
						"type": "uint32",
						"name": "menuItems",
						"id": 3
					},
					{
						"rule": "optional",
						"type": "string",
						"name": "name",
						"id": 4
					}
				]
			},
			{
				"name": "PackA4",
				"fields": [
					{
						"rule": "optional",
						"type": "bool",
						"name": "success",
						"id": 1
					},
					{
						"rule": "optional",
						"type": "string",
						"name": "orderToken",
						"id": 2
					}
				]
			},
			{
				"name": "PackB0",
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
				"name": "PackB1",
				"fields": [
					{
						"rule": "repeated",
						"type": "uint32",
						"name": "itemID",
						"id": 1
					},
					{
						"rule": "repeated",
						"type": "string",
						"name": "itemName",
						"id": 2
					},
					{
						"rule": "repeated",
						"type": "float",
						"name": "itemPrice",
						"id": 3
					}
				]
			},
			{
				"name": "PackH0",
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
			}
		]);
		client.builder.reset();
		var packSetup = client.builder.build("MenuPacks");
		this.PackH1 = packSetup.PackH1;
		this.PackA0 = packSetup.PackA0;
		this.PackA4 = packSetup.PackA4;
		this.PackB0 = packSetup.PackB0;
		this.PackB1 = packSetup.PackB1;
		this.PackH0 = packSetup.PackH0;
	};
	this.initPacks();

	this.addOrder = function (packA0, menuItemID, amount) {
		var total = ((menuItemID & 0xff) << 8) | (amount & 0xff);
		packA0.menuItems.push(total);
	};
}

var client = new Client();

var menuManager = new MenuManager();

client.tcpConnection.onopen = function () {
	if (navigator.geolocation) {
		navigator.geolocation.getCurrentPosition(sendH0, handleError);
	}
	else {
		console.log("no geo");
	}
};

function sendA0(coordinates) {
	var packA0 = new menuManager.PackA0(coordinates.coords.latitude, coordinates.coords.longitude);
	for (let i = 0; i < itemManager.menuItems.length; i++) {
		menuManager.addOrder(itemManager.menuItems[i].id, itemManager.menuItems[i].amount);
	}
	packA0.name = $('#orderName').val();
	client.tcpConnection.sendPack(new OPacket("A0", true, [0], packA0));
}

function sendH0(coordinates) {
	var packH0 = new menuManager.PackH0(coordinates.coords.latitude, coordinates.coords.longitude);
	client.tcpConnection.sendPack(new OPacket("H0", true, [0], packH0));
}

function handleError2(error) {
	switch (error.code) {
		case error.PERMISSION_DENIED:
			//TEMP IMPLEMENTATION
			var packA0 = new menuManager.PackA0(coordinates.coords.latitude, coordinates.coords.longitude);
			for (let i = 0; i < itemManager.menuItems.length; i++) {
				menuManager.addOrder(itemManager.menuItems[i].id, itemManager.menuItems[i].amount);
			}
			packA0.name = $('#orderName').val();
			client.tcpConnection.sendPack(new OPacket("A0", true, [0], packA0));
			console.log("permission denied");
			break;
		case error.POSITION_UNAVAILABLE:
			console.log("no position");
			break;
		case error.TIMEOUT:
			console.log("to");
			break;
		case error.UNKNOWN_ERROR:
			console.log("unknown");
			break;
	}
	//window.location.href = "noGeo.html";
}

function handleError(error) {
	switch (error.code) {
		case error.PERMISSION_DENIED:
			//TEMP IMPLEMENTATION
			var packH0 = new menuManager.PackH0(33.861071, -118.402267);
			client.tcpConnection.sendPack(new OPacket("H0", true, [0], packH0));
			console.log("permission denied");
			break;
		case error.POSITION_UNAVAILABLE:
			console.log("no position");
			break;
		case error.TIMEOUT:
			console.log("to");
			break;
		case error.UNKNOWN_ERROR:
			console.log("unknown");
			break;
	}
	//window.location.href = "noGeo.html";
}

client.tcpConnection.onclose = function () {
	window.location.href = "noServer.html";
};

client.packetManager.addPKey(new PKey("A4", function (iPack) {
	var packA4 = menuManager.PackA4.decode(iPack.packData);
	if (packA4.success) {
		console.log("order acknowledged");
		localStorage.setItem("orderToken", packA4.orderToken);
	}
}, this, "Gets the success of the login"));

client.packetManager.addPKey(new PKey("B1", function (iPack) {
	var packB1 = menuManager.PackB1.decode(iPack.packData);
	if (packB1.itemID !== undefined) {
		var menuItems = new Array();
		for (let i = 0; i < packB1.itemID.length; i++) {
			menuItems[i] = new MenuItem(packB1.itemID[i], packB1.itemName[i], packB1.itemPrice[i]);
		}
		itemManager = new ItemManager(menuItems);
		$('#loading').addClass("hidden");
		$('#reviewOrder').removeClass("hidden");
	}
}, this, "Gets the success of the login"));

client.packetManager.addPKey(new PKey("H1", function (iPack) {
	var packH1 = menuManager.PackH1.decode(iPack.packData);
	if (packH1.state === 1) {
		var packB0 = new menuManager.PackB0(true);
		client.tcpConnection.sendPack(new OPacket("B0", true, [0], packB0));
	}
	else {
		redirect("outRange.html");
	}
}, this, "Gets the success of the login"));