'use strict';

var WEBSOCKET_DOMAIN = "wss://beachbevs.com";
var WEBSOCKET_PORT = "8443";

function Redirect(url) {
	window.location.href = url;
}

var setman = null;

function SetupManager(loginRequired, manager) {
	this._checkManager = function () {
		if (setman._manager.onProto == null) {
			console.warn("Missing onProto call");
		}
		if (setman._manager.onOpen == null) {
			console.warn("Missing onOpen call");
		}
		if (setman._manager.onReopen == null) {
			console.warn("Missing onReopen call");
		}
		if (setman._manager.onClose == null) {
			console.warn("Missing onClose call");
		}
	}

	this._openCaller = function() {
		if (!setman._opened) {
			if (setman._manager.onOpen != null) {
				setman._manager.onOpen();
			}
			setman._opened = true;
		}
		else {
			if (setman._manager.onReopen != null) {
				setman._manager.onReopen();
			}
		}
		setman.mbarManager.hideNoServer();
	}

	this._startClient = function () {
		setman.client = new Client(WEBSOCKET_DOMAIN, WEBSOCKET_PORT, function (root) {
			if (setman._manager.onProto != null) {
				setman._manager.onProto();
			}
			setman.client.tcpConnection.onopen = function () {
				if (!setman._loginRequired) {
					setman._openCaller();
				}
				setman.innerLoginManager = new InnerLoginManager(setman.client, setman.client.root,
					function () //onlogin
					{
						if (!setman._mbarCalled) {
							setman.mbarManager.initPacks();
							setman.mbarManager.initSubEmpDiv();
							setman._mbarCalled = true;
						}
						if (setman._loginRequired) {
							setman._openCaller();
						}
					},
					function () //onfail
					{
						if (setman._loginRequired) {
							Redirect('./login.html?' + document.location.href);
						}
					});
				setman.innerLoginManager.handleOpen();
			}

			setman.client.tcpConnection.onclose = function () {
				if (setman._manager.onClose != null) {
					setman._manager.onClose(setman._opened);
				}
				setman.mbarManager.showNoServer();
				setman.client.connect();
			}
		});
	}
	setman = this;
	setman.client = null;
	setman.innerLoginManager = null;
	setman.mbarManager = new MBarManager();

	setman._opened = false;
	setman._mbarCalled = false;
	setman._loginRequired = loginRequired;
	setman._manager = manager;
	setman._checkManager();
	setman._startClient();
}
