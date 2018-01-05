
function post_event(widgetId, action)
{
	var xhttp = new XMLHttpRequest();

	xhttp.onreadystatechange = function()
	{
		if (xhttp.readyState == 4 && xhttp.status == 200)
		{
			var response = xhttp.responseText;
			var j = JSON.parse(response);

			for (var i in j)
			{
				var elm = document.getElementById(i);
				if (elm != null)
				{
					if (elm.__processDelta__ != null)
						elm.__processDelta__(j[i]);
					else
						elm.innerHTML = j[i];
				}
			}
		}
	}

	xhttp.open("GET", "event?wid=" + widgetId + "&wa=" + action, true);
	xhttp.send();
}

function init_ace_textarea(id)
{
	var editor = ace.edit("_" + id);
	editor.setTheme("ace/theme/monokai");
	editor.getSession().setMode("ace/mode/c_cpp");
	editor.getSession().on("change", function(e) { post_event(id, 2); });

	var elm = document.getElementById("_" + id);
	elm.__processDelta__ = function(dt)
	{
		editor.setValue(dt, -1);
	};

	elm.style.fontSize = "10px";
}