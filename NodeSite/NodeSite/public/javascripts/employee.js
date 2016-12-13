"use strict";

console.log("loaded");

function ApplyManager() {
  this.initPacks = function () {
    client.builder.define("ApplyPacks");
    client.builder.create([
      {
        "name": "PackE0",
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
            "name": "pwd",
            "id": 2
          },
          {
            "rule": "optional",
            "type": "string",
            "name": "email",
            "id": 3
          }
        ]
      },
      {
        "name": "PackE1",
        "fields": [
          {
            "rule": "optional",
            "type": "string",
            "name": "msg",
            "id": 1
          }
        ]
      }
    ]);
    client.builder.reset();
    var packSetup = client.builder.build("ApplyPacks");
    this.PackE0 = packSetup.PackE0;
    this.PackE1 = packSetup.PackE1;
  };
  this.initPacks();

  client.packetManager.addPKey(new PKey("E1", function (iPack) {
    var packD1 = applyManager.PackE1.decode(iPack.packData);
    $('#msg').html(packD1.msg);
  }, this, "Gets the success of the login"));

  this.submit = function () {
    if ($('#userName').val().length === 0) {
      $('#msg').text("No username was entered");
      return;
    }
    if ($('#pwd').val().length < 8) {
      $('#msg').text("Password must be longer than 8 characters");
      return;
    }
    if ($('#pwd').val() != $('#pwdConfirm').val()) {
      $('#msg').text("Confirm password does not match password");
      return;
    }
    if (!String($('#email').val()).includes('@')) {
      $('#msg').text("Invalid email");
      return;
    }
    var packE0 = new applyManager.PackE0($('#userName').val(), $('#pwd').val(), $('#email').val());
    client.tcpConnection.sendPack(new OPacket("E0", true, [0], packE0));
  };

  $('#applyButton').click(this.submit);
};

var client = new Client("localhost", "24560");

client.tcpConnection.onclose = function () {
  alert("The Server Is Unavailible...");
};

var applyManager = new ApplyManager();