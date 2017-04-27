'use strict';

function PKeyButton(jKey, onClick) {
	this.addClass = function (jClass) {
		$(this._jKey).addClass(jClass);
	}
	this.removeClass = function (jClass) {
		$(this._jKey).removeClass(jClass);
	}
	this.text = function (text, innerID) {
		$(this._jKey + '>' + innerID).html(text);
	}

	this.bind = function () {
		if (!this._bound) {
			$(this._jKey).click(this._onClick);
			this._bound = true;
			return true;
		}
		return false;
	}
	this.unbind = function () {
		if (this._bound) {
			$(this._jKey).unbind('click');
			this._bound = false;
			return true;
		}
		return false;
	}

	this.isBound = function () {
		return this._bound;
	}

	this._jKey = jKey;
	this._onClick = onClick;
	this._bound = false;
	this.bind();
}