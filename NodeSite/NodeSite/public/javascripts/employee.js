"use strict";

function setErrorMsg (str) {
  $('#msg').removeClass('invisible');
  $('#msg').text(str);
  $('#applyButton').addClass('error');
  $('#applyButton').removeClass('load');
  $('#applyButton h2').text('ERROR');
}

$('#userName, #pwd, #pwdConfirm, #email').focus(function () {
  if ($('#applyButton').hasClass('error')) {
    $('#applyButton').removeClass('error');
    $('#msg').addClass('invisible');
    $('#applyButton h2').text('APPLY NOW');
  }
});

function ApplyManager(root) {
  
  this.PackE0 = root.lookup("ProtobufPackets.PackE0");
  this.PackE1 = root.lookup("ProtobufPackets.PackE1");

  client.packetManager.addPKey(new PKey("E1", function (iPack) {
    var packE1 = applyManager.PackE1.decode(iPack.packData);
    if (!packE1.success) {
      $('#applyButton').click(applyManager.submit);
      setErrorMsg(packE1.msg);
    }
    else {

      Cookies.set('creationToken', packE1.creationToken, { expires: 1, path: '/', domain: 'beachbevs.com', secure: true });
    }
  }, this, "Gets the success of the login"));

  this.submit = function () {
    console.log("submit called");
    if ($('#userName').val().length === 0) {
      setErrorMsg("No username was entered");
    }
    else if ($('#pwd').val().length < 8) {
      setErrorMsg("Password must be longer than 8 characters");
    }
    else if ($('#pwd').val() != $('#pwdConfirm').val()) {
      setErrorMsg("Confirm password does not match password");
    }
    else if (!String($('#email').val()).includes('@')) {
      setErrorMsg("Invalid email");
    }
    else {
      var packE0 = applyManager.PackE0.create({ userName: $('#userName').val(), pwd: $('#pwd').val(), email: $('#email').val() });
      client.tcpConnection.sendPack(new OPacket("E0", true, [0], packE0, applyManager.PackE0));
      $('#msg').addClass('invisible');
      $('#applyButton').removeClass('error');
      $('#applyButton').addClass('load');
      $('#applyButton h2').text('PROCESSING');
      $('#applyButton').unbind('click');
    }
  };
  $('#applyButton').click(this.submit);
};

var applyManager;

var client = new Client(function (root) {
  console.log("ON LOAD CALLED");
  applyManager = new ApplyManager(root);
  client.tcpConnection.onclose = function () {
    alert("The Server Is Unavailible...");
  };
});
