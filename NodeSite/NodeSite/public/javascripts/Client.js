function Client(ip, port)
{
    var client = this;
    var ProtoBuf = dcodeIO.ProtoBuf;
    this.builder = ProtoBuf.newBuilder();
    if (window.location.protocol != "https:")
    {
        this.tcpConnection = new TCPConnection(this.builder, "192.168.1.17", "8000");
    }
    else
    {
        this.tcpConnection = new TCPConnection(this.builder, "192.168.1.17", "8443");
    }
    this.packetManager = new PacketManager();
    this.tcpConnection.onmessage = function(iPack){client.packetManager.processPacket(iPack)};
    this.setupManager = new SetupManager();
    this.setupManager.initPacks(this);
}
