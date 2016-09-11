zPerformTrackingEmbed();

function zPerformTrackingEmbed() {
	
	var zpElems = zS('.zift_plugin');

	for (var i=0; i<zpElems.length; i++) {
		var zpElem = zpElems[i];

		if (zpElem != null) {
			var widgetId = zpElem.getAttribute('id');

			if (widgetId != null) {
				var jsInclude = "http://widgets.ziftsolutions.com/" + zBaseKey + widgetId;
				var script = document.createElement('script');
				script.type = 'text/javascript';
				script.src = jsInclude;

				var div = document.createElement('div');
				div.id = 'zift_tmp_' + widgetId;
				div.style.display = "none";
				document.body.appendChild(div);
				div.appendChild(script);
				//window.setTimeout(function() {zpElem.innerHTML = div.innerHTML;}, 1000);
			}
		}
	}
}
