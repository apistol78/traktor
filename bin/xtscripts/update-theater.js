
function quat_from_axis_angle(aa)
{
	var a = Math.sqrt(aa[0] * aa[0] + aa[1] * aa[1] + aa[2] * aa[2]);
	if (Math.abs(a) <= 0.0001)
		return [0, 0, 0, 1];
	else
	{
		var ha = a / 2.0;
		return [
			aa[0] * Math.sin(ha) / a,
			aa[1] * Math.sin(ha) / a,
			aa[2] * Math.sin(ha) / a,
			Math.cos(ha)
		];
	}
}

function quat_normalize(q)
{
	var m = Math.sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	if (m <= 0.0001)
		return null;
	else
		return [
			q[0] / m,
			q[1] / m,
			q[2] / m,
			q[3] / m
		];
}

function quat_conjugate(q)
{
	return [ -q[0], -q[1], -q[2], q[3] ];
}

function quat_mul(q1, q2)
{
	return [
		q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1],
		q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2],
		q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0],
		q1[3] * q2[3] - q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2]
	];
}

function quat_transform(q, v)
{
	var qv = [ v[0], v[1], v[2], 0 ];
	return quat_mul(quat_mul(q, qv), quat_conjugate(q));
}

function print_v(v)
{
	print(v[0].toFixed(1) + ", " + v[1].toFixed(1) + ", " + v[2].toFixed(1) + ", " + v[3].toFixed(1));
}

function transform0(current)
{
	var type = current.getAttribute("type", "").getValue();
	if (type == "traktor.theater.TrackData")
	{
		var xkeys = current.getSingle("path/keys");
		
		var h0 = 0, p0 = 0;
		
		for (var xitem = xkeys.getFirstChild(); xitem != null; xitem = xitem.getNextSibling())
		{
			var xorientation = xitem.getSingle("value/orientation");

			var qv = xorientation.getValue().split(",");
			var q = quat_normalize([
				parseFloat(qv[0]),
				parseFloat(qv[1]),
				parseFloat(qv[2]),
				parseFloat(qv[3])
			]);
			
			var h, p;
			
			if (q != null)
			{
				var az = [ 0, 0, 1, 0 ];
				var az_2 = quat_transform(q, az);
				
				h = Math.atan2(az_2[0], az_2[2]);
				
				var qnh = quat_from_axis_angle([0, -h, 0]);
				var az_3 = quat_transform(qnh, az_2);
				
				p = -Math.atan2(az_3[1], az_3[2]);
			}
			else
			{
				print("Invalid quaternion; preserving orientation from previous key");
				h = h0;
				p = p0;
			}
			
/*
			if (h0 < 0)
			{
				while (h > 0)
					h -= Math.PI * 2;
			}
			else
			{
				while (h < 0)
					h += Math.PI * 2;
			}
			if (p0 < 0)
			{
				while (p > 0)
					p -= Math.PI * 2;
			}
			else
			{
				while (p < 0)
					p += Math.PI * 2;
			}
*/

			print("JS:  " + h + ", " + p);
			print();
			
			h0 = h;
			p0 = p;
			
			h = Math.round(h * 10000) / 10000;
			p = Math.round(p * 10000) / 10000;
			
			var xv = new traktor.xml.Text(
				h + ", " + p + ", 0, 0"
			);
			
			xorientation.removeAllChildren();
			xorientation.addChild(xv);
		}
	}
}
