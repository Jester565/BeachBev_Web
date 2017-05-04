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
		homeManager.setBeachDivFontSize();
		$(window).resize(homeManager.setBeachDivFontSize);
	}


	this.setBeachDivFontSize = function () {
		var fontSize = $('#homeDiv').width() / 35.0;
		$("#stepDiv").css("margin-top", String(fontSize * 1.4 - 15) + 'px');
		$("#stepDiv").css("margin-right", String(fontSize) + 'px');
		$("#stepDiv").css("fontSize", String(fontSize * 1.2) + 'px');
		$("#step4").css("fontSize", String(fontSize * 3) + 'px');
		$('#step4').css("margin-top", '-' + String((fontSize * 3) * 1.5) + 'px');
	}
}