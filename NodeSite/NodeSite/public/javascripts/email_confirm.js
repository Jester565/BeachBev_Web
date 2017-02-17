"use strict";

function EmailConfirmManager(root) {
		emailConfirmManager = this;
  emailConfirmManager.PacketB2 = root.lookup("ProtobufPackets.PackB2");
  emailConfirmManager.PacketB3 = root.lookup("ProtobufPackets.PackB3");

  client.packetManager.addPKey(new PKey("B3", function (iPack) {
    $('#loading').addClass('hidden');
    var packB3 = emailConfirmManager.PacketB3.decode(iPack.packData);
    if (packB3.success) {
      console.log("Success!");
      $('#checkmark').removeClass('hidden');
      $('#msg').removeClass('hidden');
      $('#msg').addClass('success');
      $('#msg').text(packB3.msg);
    }
    else {
      emailConfirmManager.setErrorMsg(packB3.msg);
    }
  }, emailConfirmManager, "Gets if the confirmation was successful"));

	 emailConfirmManager.setErrorMsg = function (str) {
				$('#msg').removeClass('hidden');
				$('#msg').text(str);
		};

		var url = window.location.href;
  var questionI = url.indexOf('?');
  if (questionI !== -1) {
			emailConfirmManager.emailToken = url.substring(++questionI);
			console.log("Email Token: " + emailConfirmManager.emailToken);
				var packB2 = emailConfirmManager.PacketB2.create({
						emailToken: emailConfirmManager.emailToken
				});
				client.tcpConnection.sendPack(new OPacket("B2", true, [0], packB2, emailConfirmManager.PacketB2));
		} else {
				emailConfirmManager.setErrorMsg("No email token in url");
		}
		this.setErrorMsg = function(msg) {
				$('#msg').html(msg);
				$('#msg').removeClass('hidden');
		};
}

var emailConfirmManager;
var innerLoginManager;

var client = new Client(function (root) {
  console.log("ON LOAD CALLED");
  innerLoginManager = new InnerLoginManager(client, root,
				function () {
						console.log("LOGGED IN");
						emailConfirmManager = new EmailConfirmManager(client.root);
				});
  client.tcpConnection.onclose = function () {
     redirect('./noServer.html');
  };
});
