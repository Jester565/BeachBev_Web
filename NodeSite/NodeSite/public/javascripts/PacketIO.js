'use strict';

function OPacket(locKey, serverRead, sendToIDs, pack, packBuilder) {
	this.locKey = locKey;
	if (serverRead !== undefined) {
		this.serverRead = serverRead;
	}
	else {
		this.serverRead = true;
	}
	this.sendToIDs = sendToIDs;
	this.pack = pack;
	this.packBuilder = packBuilder;
	this.toString = function () {
		var str = "locKey: " + this.locKey + " serverRead: ";
		if (this.serverRead !== undefined) {
			if (this.serverRead) {
				str += "true";
			}
			else {
				str += "false";
			}
		}
		else {
			str += "undefined";
		}
		str += " sendToIDs: ";
		if (this.sendToIDs !== undefined) {
			if (this.sendToIDs.length !== 0) {
				for (var i = 0; i < this.sendToIDs.length; i++) {
					str += this.sendToIDs[i];
					if (i < this.sendToIDs.length - 1) {
						str += ", ";
					}
				}
			}
		}
		else {
			str += "undefined";
		}
		str += " pack: ";
		if (this.pack !== undefined) {
			str += pack;
		}
		else {
			str += "undefined";
		}
		return str;
	};
}

function IPacket(locKey, sentFromID, packData) {
	this.locKey = locKey;
	this.sentFromID = sentFromID;
	this.packData = packData;
	this.toString = function () {
		var str = "locKey: " + this.locKey;
		str += " sentFromID: " + this.sentFromID + " packData: ";
		if (this.packData !== undefined) {
			for (var i = 0; i < this.packData.length; i++) {
				str += this.packData[i];
			}
		}
		else {
			str += 'undefined';
		}
		return str;
	};
	this.toOPack = function (locKey, serverRead, sendToIDs, packData) {
		if (locKey === undefined)
			locKey = this.locKey;
		if (sendToIDs === undefined)
			sendToIDs = this.sentFromID;
		if (serverRead === undefined)
			serverRead = (this.sentFromID === 0);
		if (packData === undefined)
			packData = this.packData;
		return new OPacket(locKey, serverRead, sendToIDs, packData);
	}
}