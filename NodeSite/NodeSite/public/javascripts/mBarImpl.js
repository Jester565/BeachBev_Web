if (typeof redirect === "undefined") {
	function redirect(url) {
		window.location.href = url;
	}
}

$(document).ready(function () {
	$('#sideMenuButton').click(function () {
		$(this).toggleClass('open');
		$('#sideMenuNav').toggleClass('open');
		$('#headerRectBorderCover').toggleClass('open');
	});

	function empMenuClick() {
	}

	$('#dropArrowImg').click(function () {
		if ($('#employeeMenuDiv').hasClass('hidden')) {
			$('#employeeMenuDiv').removeClass('hidden');
			$('#dropArrowImg').addClass('dropped');
		}
		else {
			$('#employeeMenuDiv').addClass('hidden');
			$('#dropArrowImg').removeClass('dropped');
		}
	});

	$('#homeLink').click(function () {
		redirect("./index.html");
	});

	$('#employeeMenuTitle').click(function () {
		redirect("./employee.html");
	});

	$('#loginLink').click(function () {
		redirect("./login.html");
	});

	$('#applyLink').click(function () {
		redirect("./apply.html");
	});

	$('#subEmpLogOut').click(function () {
		Cookies.remove('eID');
		Cookies.remove('deviceID');
		Cookies.remove('pwdToken');
		redirect("./login.html");
	});

	$('#subEmpHome').click(function () {
		redirect("employee.html");
	});

	$('#subEmpEmail').click(function () {
		redirect("./email.html");
	});

	$('#subEmpResume').click(function () {
		redirect("./resume.html");
	});

	$('#subEmpMaster').click(function () {
		redirect("./master.html");
	});
});

var dotInterval = null;

function ShowNoServer() {
	$('#noServerDiv').removeClass('hidden');
	if (dotInterval === null) {
		dotInterval = setInterval(function () {
			var dotText = $('#noServerDiv > p > a').text();
			if (dotText.length < 3) {
				dotText += ".";
			}
			else {
				dotText = "";
			}
			$('#noServerDiv > p > a').text(dotText);
		}, 1000);
	}
}

function HideNoServer() {
	$('#noServerDiv').addClass('hidden');
	if (dotInterval !== null) {
		clearInterval(dotInterval);
		dotInterval = null;
	}
}