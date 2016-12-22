"use strict";

console.log("loaded");

function ApplyManager() {
  $('#userName, #pwd, #pwdConfirm, #email').focus(function () {
    if ($('#applyButton').hasClass('error')) {
      $('#applyButton').removeClass('error');
      $('#msg').addClass('invisible');
      $('#applyButton h2').text('APPLY NOW');
    }
  });

  this.setErrorMsg = function (str) {
    $('#msg').removeClass('invisible');
    $('#msg').text(str);
    $('#applyButton').addClass('error');
    $('#applyButton').removeClass('load');
    $('#applyButton h2').text('ERROR');
  }

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
    var packSetup = client.builder.build("ApplyPacks");
    this.PackE0 = packSetup.PackE0;
    this.PackE1 = packSetup.PackE1;
  };

  this.initPacks();

  client.packetManager.addPKey(new PKey("E1", function (iPack) {
    var packE1 = applyManager.PackE1.decode(iPack.packData);
    if (!packE1.success) {
      $('#applyButton').click(applyManager.submit);
      applyManager.setErrorMsg(packE1.msg);
    }
    else {
      var fadeTime = 400;
      $('#applyDiv ul').addClass('hidden');
      $('#successMsg').removeClass('hidden');
      $('#successMsg').text(packE1.msg);
      $('#msg').removeClass('invisible');
      $('#msg').addClass('success');
      $('#msg').text("To verify your email you must click the link in the email sent by management@beachbevs.com\
        If this is not done in 2 hours, your account will be deleted. During the 2 hour period, you can log into your account\
        to access a limited number of services.");
      $('#applyButton').removeClass('load');
      $('#applyButton h2').text("Resend email");
      $('#applyButton').click(function () {
        console.log("Resent email");
      });
    }
  }, this, "Gets the success of the login"));

  this.submit = function () {
    console.log("submit called");
    if ($('#userName').val().length === 0) {
      applyManager.setErrorMsg("No username was entered");
    }
    else if ($('#pwd').val().length < 8) {
      applyManager.setErrorMsg("Password must be longer than 8 characters");
    }
    else if ($('#pwd').val() != $('#pwdConfirm').val()) {
      applyManager.setErrorMsg("Confirm password does not match password");
    }
    else if (!String($('#email').val()).includes('@')) {
      applyManager.setErrorMsg("Invalid email");
    }
    else {
      var packE0 = new applyManager.PackE0($('#userName').val(), $('#pwd').val(), $('#email').val());
      client.tcpConnection.sendPack(new OPacket("E0", true, [0], packE0));
      $('#msg').addClass('invisible');
      $('#applyButton').removeClass('error');
      $('#applyButton').addClass('load');
      $('#applyButton h2').text('PROCESSING');
      $('#applyButton').unbind('click');
    }
  };
  $('#applyButton').click(this.submit);
};

var client = new Client("localhost", "24560");

client.tcpConnection.onclose = function () {
  alert("The Server Is Unavailible...");
};

var applyManager = new ApplyManager();