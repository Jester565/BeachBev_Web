$(document).ready(function () {
	$('#sideMenuButton').click(function () {
		$(this).toggleClass('open');
		$('#sideMenuNav').toggleClass('open');
		$('#headerRectBorderCover').toggleClass('open');
	});
});