function SetupManager() {
	var setupManager = this;
	this.initPacks = function (client) {
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
	}
}