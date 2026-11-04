/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// Application location is kept in the URL fragment; start view has none.
function current_location()
{
	try
	{
		return decodeURIComponent(location.hash.substring(1));
	}
	catch (e)
	{
		return "";
	}
}

function location_url(loc)
{
	return loc == "" ? location.pathname + location.search : "#" + encodeURI(loc);
}

function send_event(body, navigating)
{
	// Send values of all inputs with every event.
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

			// New locations become history entries; navigation only corrects the current one.
			var loc = j["_location"];
			if (loc != null && loc != current_location())
			{
				if (navigating)
					history.replaceState(null, "", location_url(loc));
				else
					history.pushState(null, "", location_url(loc));
			}
		}
	}

	xhttp.open("POST", "event", true);
	xhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
	xhttp.setRequestHeader("X-Webber", "1");
	xhttp.send(body);
}

function post_event(widgetId, action)
{
	send_event("wid=" + widgetId + "&wa=" + action, false);
}

function post_navigate(loc)
{
	send_event("nav=" + encodeURIComponent(loc), true);
}

window.addEventListener("popstate", function()
{
	post_navigate(current_location());
});

window.addEventListener("DOMContentLoaded", function()
{
	var meta = document.querySelector("meta[name='webber-location']");
	if (meta != null && meta.content != current_location())
		post_navigate(current_location());
});
