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
});