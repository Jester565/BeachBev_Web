'use strict';

function PKey(locKey, func, obj, description)
{
    this.obj = obj;
    this.locKey = locKey;
    this.func = func;
    this.description = description;
}

function PacketManager()
{
    this.pKeys = new Map();
    this.addPKey = function(pKey) 
    {
        if(pKey !== undefined){
            this.pKeys.set(pKey.locKey, pKey);
            console.log("pKey: " + pKey.locKey + " was set");
        }           
    }
    this.processPacket = function(iPack)
    {
        var pKey = this.pKeys.get(iPack.locKey);
        if (pKey !== undefined)
        {
            pKey.func(iPack);
        }
        else
        {
            console.log("The key " + iPack.locKey + " was not found in pKeys, locKey Size: " + iPack.locKey.length);
            alert(JSON.stringify(this.pKeys, null, 4));
        }
    }
    this.removePKey = function(pKey)
    {
        console.log("REMOVE CALLED");
        if(typeof pKey === "string")
        {
            this.pKeys.delete(pKey);
        }
        else
        {
            this.pKeys.delete(pKey.locKey);
        }
    }
}

