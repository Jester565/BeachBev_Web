"use strict";

function setErrorMsg(str) {
  $('#msg').removeClass('hidden');
  $('#msg').text(str);
  $('#confirmButton').addClass('error');
  $('#confirmButton').removeClass('load');
  $('#confirmButton h2').text('ERROR');
}

function EmailConfirmManager(root) {
  this.PackI0 = root.lookup("ProtobufPackets.PackI0");
  this.PackI1 = root.lookup("ProtobufPackets.PackI1");
  this.PackI2 = root.lookup("ProtobufPackets.PackI2");

  client.packetManager.addPKey(new PKey("I2", function (iPack) {
    $('#loading').addClass('hidden');
    var packI2 = emailConfirmManager.PackI2.decode(iPack.packData);
    if (packI2.success) {
      console.log("Success!");
      $('#login').addClass('hidden');
      $('#checkmark').removeClass('hidden');
      $('#checkmark').addClass('animate');
      $('#msg').addClass('success');
      $('#msg').text(packI2.msg);
    }
    else {
      if (packI2.requestI1) {
        $('#login').removeClass('hidden');
      }
      setErrorMsg(packI2.msg);
    }
  }, this, "Gets the success of the login"));
}

var emailConfirmManager;

var client = new Client(function (root) {
  emailConfirmManager = new EmailConfirmManager(root);
  client.tcpConnection.onopen = function () {
    var url = window.location.href;
    var questionI = url.indexOf('?');
    if (questionI !== -1) {
      emailConfirmManager.emailToken = url.substring(++questionI);
      var creationToken = Cookies.get('creationToken');
      console.log("CreationToken: " + creationToken);
      if (creationToken !== undefined) {
        var packI0 = emailConfirmManager.PackI0.create({ emailToken: emailConfirmManager.emailToken, creationToken: creationToken });
        console.log(packI0.creationToken);
        client.tcpConnection.sendPack(new OPacket("I0", true, [0], packI0, emailConfirmManager.PackI0));
      }
      else {
        $('#confirmButton').click(function () {
          if ($('#userName').val().length === 0) {
            setErrorMsg('Name not entered');
          }
          else if ($('#pwd').val().length === 0) {
            setErrorMsg('Password not entered');
          }
          else {
            $('#confirmButton').addClass('load');
            var packI1 = emailConfirmManager.PackI1.create({ emailToken: emailConfirmManager.emailToken, userName: $('#userName').val(), pwd: $('#pwd').val() });
            client.tcpConnection.sendPack(new OPacket("I1", true, [0], packI1, emailConfirmManager.PackI1));
          }
        });

        $('#userName, #pwd').focus(function () {
          if ($('#confirmButton').hasClass('error')) {
            $('#confirmButton').removeClass('error');
            $('#msg').addClass('hidden');
            $('#confirmButton h2').text('CONFIRM');
          }
        });
        $('#loading').addClass('hidden');
        $('#login').removeClass('hidden');
      }
    }
    else {
      $('#loading').addClass('hidden');
      setErrorMsg('Invalid URL: No token');
    }
  };
});
