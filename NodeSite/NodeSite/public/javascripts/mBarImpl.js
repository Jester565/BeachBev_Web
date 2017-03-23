function redirect(url) {
	window.location.href = url;
}

$(document).ready(function () {
	$('#sideMenuButton').click(function () {
		$(this).toggleClass('open');
		$('#sideMenuNav').toggleClass('open');
		$('#headerRectBorderCover').toggleClass('open');
	});

	$('#homeLink').click(function () {
		redirect("./index.html");
	});

	$('#employeeLink').click(function () {
		redirect("./employee.html");
	});
});