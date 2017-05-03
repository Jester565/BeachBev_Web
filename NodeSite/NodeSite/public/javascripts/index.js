'use strict';


var setman = null;
var homeManager = null;
$(document).ready(function () {
	$("#mBar").load("./mBar.html", function () {
		homeManager = new HomeManager();
		setman = new SetupManager(false, homeManager);
	});
});

function HomeManager() {
	homeManager = this;

	this.onOpen = function () {
		$('#ordernow').click(function () {
			Redirect("menu.html");
		});
	}
}