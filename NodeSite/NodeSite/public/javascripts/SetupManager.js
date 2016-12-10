
function SetupManager()
{
    var setupManager = this;
    this.initPacks = function(client)
    {
        client.builder.define("CSetUpPacks");
        client.builder.create([
            {
                "name": "PackA0",
                "fields": [
                    {
                        "rule": "optional",
                        "type": "string",
                        "name": "username",
                        "id": 1
                    }
                ] 
            },
            {
                "name": "PackA1",
                "fields": [
                    {
                        "rule": "optional",
                        "type": "uint32",
                        "name": "id",
                        "id": 1
                    }
                ]
            },
            {
                "name": "PackA2",
                "fields": [
                    {
                        "rule": "optional",
                        "type": "uint32",
                        "name": "id",
                        "id": 1
                    },
                    {
                        "rule": "optional",
                        "type": "string",
                        "name": "username",
                        "id": 2
                    }
                ]
            },
            {
                "name": "PackA3",
                "fields": [
                    {
                        "rule": "optional",
                        "type": "uint32",
                        "name": "id",
                        "id": 1
                    },
                    {
                        "rule": "optional",
                        "type": "string",
                        "name": "username",
                        "id": 2
                    }
                ]
            }
        ]);
        client.builder.reset();
         var packSetup = client.builder.build("CSetUpPacks");
        this.PackA0 = packSetup.PackA0;
        this.PackA1 = packSetup.PackA1;
        this.PackA2 = packSetup.PackA2;
        this.PackA3 = packSetup.PackA3;
        /*
        client.packetManager.addPKey(new PKey("A1", function(iPack){
            var packA1 = this.obj.PackA1.decode(iPack.packData);
            client.id = packA1.id;
        }, this, "Recieves the ID of this client from the server"));
         client.tcpConnection.onopen = function(){
            var packA0 = new setupManager.PackA0("Jester565");
            client.tcpConnection.sendPack(new OPacket("A0", true, [0], packA0));
        }
        */
    }
}