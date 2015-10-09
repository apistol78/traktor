#include <cmath>
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Bezier3rd.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Ray2.h"

namespace traktor
{

Bezier3rd::Bezier3rd()
{
}

Bezier3rd::Bezier3rd(const Vector2& cp0_, const Vector2& cp1_, const Vector2& cp2_, const Vector2& cp3_)
:	cp0(cp0_)
,	cp1(cp1_)
,	cp2(cp2_)
,	cp3(cp3_)
{
}

Vector2 Bezier3rd::evaluate(float t) const
{
	const Vector2 D = cp0;
	const Vector2 C = 3.0f * cp1 - 3.0f * cp0;
	const Vector2 B = 3.0f * cp2 - 6.0f * cp1 + 3.0f * cp0;
	const Vector2 A = cp3 - 3.0f * cp2 + 3.0f * cp1 - cp0;
	return (t * t * t) * A + (t * t) * B + t * C + D;
}

Vector2 Bezier3rd::tangent(float t) const
{
	const Vector2 C = 3.0f * cp1 - 3.0f * cp0;
	const Vector2 B = 3.0f * cp2 - 6.0f * cp1 + 3.0f * cp0;
	const Vector2 A = cp3 - 3.0f * cp2 + 3.0f * cp1 - cp0;
	return (3.0f * t * t) * A + (2.0f * t) * B + C;
}

void Bezier3rd::split(float t, Bezier3rd& outLeft, Bezier3rd& outRight) const
{
	Vector2 p = evaluate(t);
	Vector2 c_12 = lerp(cp0, cp1, t);
	Vector2 c_23 = lerp(cp1, cp2, t);
	Vector2 c_34 = lerp(cp2, cp3, t);
	Vector2 c_123 = lerp(c_12, c_23, t);
	Vector2 c_234 = lerp(c_23, c_34, t);
	outLeft = Bezier3rd(
		cp0,
		c_12,
		c_123,
		p
	);
	outRight = Bezier3rd(
		p,
		c_234,
		c_34,
		cp3
	);
}

namespace
{

	void approximateSubdivide(const Bezier3rd& b, float f0, float f1, float errorThreshold, int maxSubdivisions, AlignedVector< Bezier2nd >& outQuadratic)
	{
		Vector2 p_0 = b.evaluate(f0);
		Vector2 p_1 = b.evaluate(f1);
		Vector2 p_m = b.evaluate((f0 + f1) / 2.0f);
		Vector2 cp = 2.0f * p_m - 0.5f * (p_0 + p_1);
		
		Bezier2nd b2(p_0, cp, p_1);

		float maxError = 0.0f;
		const int32_t errorSteps = 10;
		for (int32_t i = 1; i < errorSteps - 1; ++i)
		{
			float f = float(i) / (errorSteps - 1);

			Vector2 ps = b.evaluate(f0 + f * (f1 - f0));
			Vector2 pe = b2.evaluate(f);

			float error = (pe - ps).length();
			maxError = max(error, maxError);
		}

		if (outQuadratic.size() >= maxSubdivisions || maxError <= errorThreshold)
		{
			outQuadratic.push_back(b2);
			return;
		}
		else
		{
			AlignedVector< Bezier2nd > ql, qr;
			approximateSubdivide(b, f0, (f0 + f1) * 0.5f, errorThreshold, maxSubdivisions / 2, ql);
			approximateSubdivide(b, (f0 + f1) * 0.5f, f1, errorThreshold, maxSubdivisions / 2, qr);
			outQuadratic.insert(outQuadratic.end(), ql.begin(), ql.end());
			outQuadratic.insert(outQuadratic.end(), qr.begin(), qr.end());
		}
	}

}

void Bezier3rd::approximate(float errorThreshold, int maxSubdivisions, AlignedVector< Bezier2nd >& outQuadratic) const
{
	outQuadratic.resize(0);
	approximateSubdivide(*this, 0.0f, 1.0f, errorThreshold, maxSubdivisions, outQuadratic);
}

}
