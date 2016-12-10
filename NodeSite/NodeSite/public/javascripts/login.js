"use strict";

function LoginManager()
{
    this.initPacks = function()
      {
          client.builder.define("LoginPacks");
          client.builder.create([
              {
                  "name": "PackD0",
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
                      }
                  ] 
              },
              {
                "name": "PackD1",
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
                        "name": "error",
                        "id": 2
                    },
                    {
                        "rule": "optional",
                        "type": "string",
                        "name": "token",
                        "id": 3
                    }
                ]
              }
          ]);
          client.builder.reset();
          var packSetup = client.builder.build("LoginPacks");
          this.PackD0 = packSetup.PackD0;
          this.PackD1 = packSetup.PackD1;
      };
    this.initPacks();
    
    client.packetManager.addPKey(new PKey("D1", function(iPack){
            var packD1 = loginManager.PackD1.decode(iPack.packData);
            if (packD1.success) {
                var token = packD1.token;
                console.log("TOKEN: ");
                console.log(token);
                localStorage.setItem("name", $('#userName').val());
                localStorage.setItem("token", token);
                window.location.href = "orders.html";
            }
            else
            {
                $('#msg').html(packD1.error);
            }
        }, this, "Gets the success of the login"));
    
    this.login = function() {
        if ($('#userName').val().length === 0 ) {
            $('#msg').text("No username was entered");
            return;
        }
        if ($('#pwd').val().length < 8) {
            $('#msg').text("Password must be longer than 8 characters");
            return;
        }
        var packD0 = new loginManager.PackD0($('#userName').val(), $('#pwd').val());
        client.tcpConnection.sendPack(new OPacket("D0", true, [0], packD0));
    };
    
    $('#login').click(this.login);
};

var client = new Client("localhost", "24560");

client.tcpConnection.onclose = function()
{
    alert("The Server Is Unavailible...");
};

var loginManager = new LoginManager();