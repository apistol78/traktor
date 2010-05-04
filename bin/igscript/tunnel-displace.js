
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
	
	var a = Math.atan2(fy, fx) / 3.14;
	var l = 0.1 + 0.7 / Math.sqrt(fx * fx + fy * fy);
	
	return new Color(
		(a * 0.5 + 0.5),
		l,
		1.0 - l * 0.3
	);
}
