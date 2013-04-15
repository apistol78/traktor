
var g_width;
var g_height;

function initialize(width, height)
{
	g_width = width;
	g_height = height;
}

function evaluate(x, y)
{
	var fx = (x / (g_width - 1)) * 2 - 1;
	var fy = (y / (g_height - 1)) * 2 - 1;

	var r = Math.sqrt(fx * fx + fy * fy);
	if (r < 1)
	{
		var s = Math.sin(r * 3.142) * 0.8;

		if (r > 0.8)
			s = s + Math.sin((r - 0.8) * 5 * 3.142) * 0.2;

		var dx = s * (fx / r);
		var dy = s * (fy / r);

		return new Color(
			(dx * 0.5 + 0.5),
			(dy * 0.5 + 0.5),
			0,
			1
		);
	}
	else
		return new Color(
			0.5,
			0.5,
			0,
			0
		);
}
