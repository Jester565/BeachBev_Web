function Client()
{
    var client = this;
    var ProtoBuf = dcodeIO.ProtoBuf;
    this.builder = ProtoBuf.newBuilder();
    if (window.location.protocol != "https:")
    {
        this.tcpConnection = new TCPConnection(this.builder, location.host, "8000");
    }
    else
    { 
        this.tcpConnection = new TCPConnection(this.builder, location.host, "8443");
    }
    this.packetManager = new PacketManager();
    this.tcpConnection.onmessage = function(iPack){client.packetManager.processPacket(iPack)};
    this.setupManager = new SetupManager();
    this.setupManager.initPacks(this);
}
