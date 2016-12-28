
function Client(protoInitCallback)
{
  var client = this;
  protobuf.load("PackFW.proto", function (err, root) {
    if (err) {
      throw err;
    }
    if (window.location.protocol != "https:") {
      client.tcpConnection = new TCPConnection(root, location.host, "8000");
    }
    else {
      client.tcpConnection = new TCPConnection(root, location.host, "8443");
    }
    client.packetManager = new PacketManager();
    client.tcpConnection.onmessage = function (iPack) { client.packetManager.processPacket(iPack) };
    console.log("CALLED");
    protoInitCallback(root);
  });
}
