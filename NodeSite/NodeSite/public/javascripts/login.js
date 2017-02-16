"use strict";


function LoginManager(root) {
		loginManager = this;
  loginManager.PacketA3 = root.lookup("ProtobufPackets.PackA3");
  loginManager.PacketA1 = root.lookup("ProtobufPackets.PackA1");

  client.packetManager.addPKey(new PKey("A1", function (iPack) {
    var packA1 = loginManager.PacketA1.decode(iPack.packData);
    if (packA1.pwdToken === null || packA1.pwdToken.length <= 0) {
						loginManager.bindButtons();
      loginManager.setErrorMsg(packA1.msg);
    }
    else {
						Cookies.set('pwdToken', packA1.pwdToken, { expires: 1, path: '/', domain: 'beachbevs.com', secure: true });
						Cookies.set('deviceID', packA1.deviceID, { path: '/', domain: 'beachbevs.com', secure: true });
						Cookies.set('eID', packA1.eID, { path: '/', domain: 'beachbevs.com', secure: true });
						var url = document.location.href;
						var questionI = url.indexOf('?');
						if (questionI !== -1) {
								redirect(url.substring(++questionI));
						}
						else {
								redirect('./employee.html');
						}
				}
  }, loginManager, "Gets the success of the login"));

		loginManager.bindButtons = function () {
				$('#loginButton').removeClass('processing');
				$('#pwdResetButton').removeClass('processing');
				$('#loginButton').click(function () {
						if ($('#userName').val().length <= 0) {
								loginManager.setErrorMsg("No username was entered");
						}
						else if ($('#pwd').val().length <= 0) {
								loginManager.setErrorMsg("No password was entered");
						}
						else {
								var devID = Cookies.get('deviceID');
								if (devID === null) {
										devID = 0;
								}
								var packA3 = loginManager.PacketA3.create({
										name: $('#userName').val(),
										pwd: $('#pwd').val(),
										deviceID: devID
								});
								client.tcpConnection.sendPack(new OPacket("A3", true, [0], packA3, loginManager.PacketA3));
								loginManager.unbindButtons();
						}
				});
		};

		loginManager.unbindButtons = function () {
				$('#msg').addClass('hidden');
				$('#loginButton').unbind('click');
				$('#pwdResetButton').unbind('click');
				$('#loginButton').addClass('processing');
				$('#pwdResetButton').addClass('processing');
		};

		loginManager.setErrorMsg = function(msg) {
				$('#msg').text(msg);
				$('#msg').removeClass('hidden');
		}

		loginManager.bindButtons();
		$('#loginDiv').removeClass('hidden');
		$('#loading').addClass('hidden');
};

var loginManager;

var client = new Client(function (root) {
  console.log("ON LOAD CALLED");
		client.tcpConnection.onopen = function () {
				loginManager = new LoginManager(client.root);
		}
  client.tcpConnection.onclose = function () {
    redirect('./noServer.html');
  };
});

