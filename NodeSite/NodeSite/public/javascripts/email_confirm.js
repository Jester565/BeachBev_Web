"use strict";

function EmailConfirmManager(root) {
  this.PackI0 = root.lookup("ProtobufPackets.PackI0");
  this.PackI1 = root.lookup("ProtobufPackets.PackI1");
  this.PackI2 = root.lookup("ProtobufPackets.PackI2");

  client.packetManager.addPKey(new PKey("I2", function (iPack) {
    var packI2 = emailConfirmManager.PackI2.decode(iPack.packData);
    if (packI2.success) {
      console.log("Success!");
    }
    else {
      console.log("Failed: " + packI2.msg);
    }
  }, this, "Gets the success of the login"));
}

var emailConfirmManager;

var client = new Client(function (root) {
  emailConfirmManager = new EmailConfirmManager(root);
  client.tcpConnection.onopen = function () {
    var url = window.location.href;
    var questionI = url.indexOf('?');
    if (questionI != -1) {
      var emailToken = url.substring(++questionI);
      var creationToken = Cookies.get('creationToken');
      if (creationToken !== undefined) {
        var packI0 = emailConfirmManager.PackI0.create({ emailToken: emailToken, creationToken: creationToken });
        client.tcpConnection.sendPack(new OPacket("I0", true, [0], packI0, emailConfirmManager.PackI0));
      }
    }
  };

});
