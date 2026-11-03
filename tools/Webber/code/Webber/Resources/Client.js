/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

function post_event(widgetId, action)
{
	// Send values of all inputs with every event.
	var body = "wid=" + widgetId + "&wa=" + action;
	var inputs = document.querySelectorAll("input[id], textarea[id]");
	for (var i = 0; i < inputs.length; ++i)
		body += "&" + inputs[i].id + "=" + encodeURIComponent(inputs[i].value);

	var xhttp = new XMLHttpRequest();

	xhttp.onreadystatechange = function()
	{
		if (xhttp.readyState != 4)
			return;

		// Session has expired; reload to start a new one.
		if (xhttp.status == 404)
			location.reload();

		if (xhttp.status == 200)
		{
			var j = JSON.parse(xhttp.responseText);
			for (var i in j)
			{
				var elm = document.getElementById(i);
				if (elm != null)
				{
					elm.innerHTML = j[i];

					// Edited text areas no longer follow their content.
					if (elm.tagName == "TEXTAREA")
						elm.value = elm.defaultValue;
				}
			}
		}
	}

	xhttp.open("POST", "event", true);
	xhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
	xhttp.setRequestHeader("X-Webber", "1");
	xhttp.send(body);
}
