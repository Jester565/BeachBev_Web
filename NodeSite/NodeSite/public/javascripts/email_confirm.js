"use strict";

var client = new Client();

var emailConfirmManager = new EmailConfirmManager();

client.tcpConnection.onopen = function () {
  emailConfirmManager.initPacks();
  var url = window.location.href;
  var questionI = url.indexOf('?');
  if (questionI != -1) {
    var emailToken = url.substring(++questionI);
    var creationToken = Cookies.get('creationToken');
    if (creationToken !== undefined) {
      var packI0 = new emailConfirmManager.PackI0(emailToken, creationToken);
      client.tcpConnection.sendPack(new OPacket("I0", true, [0], packI0));
    }
  }
};

function EmailConfirmManager() {
  this.initPacks = function () {
    client.builder.define("EmailConfirmPacks");
    client.builder.create([
      {
        "name": "PackI0",
        "fields": [
          {
            "rule": "optional",
            "type": "string",
            "name": "emailToken",
            "id": 1
          },
          {
            "rule": "optional",
            "type": "string",
            "name": "creationToken",
            "id": 2
          }
        ]
      },
      {
        "name": "PackI1",
        "fields": [
          {
            "rule": "optional",
            "type": "string",
            "name": "emailToken",
            "id": 1
          },
          {
            "rule": "optional",
            "type": "string",
            "name": "userName",
            "id": 2
          },
          {
            "rule": "optional",
            "type": "string",
            "name": "pwd",
            "id": 3
          }
        ]
      },
      {
        "name": "PackI2",
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
            "name": "msg",
            "id": 2
          }
        ]
      }
    ]);
    client.builder.reset();
    var packSetup = client.builder.build("EmailConfirmPacks");
    this.PackI0 = packSetup.PackI0;
    this.PackI1 = packSetup.PackI1;
    this.PackI2 = packSetup.PackI2;

    client.packetManager.addPKey(new PKey("I2", function (iPack) {
      var packI2 = emailConfirmManager.PackI2.decode(iPack.packData);
      if (packI2.success) {
        console.log("Success!");
      }
      else {
        console.log("Failed: " + packI2.msg);
      }
    }, this, "Gets the success of the login"));
  };
}
