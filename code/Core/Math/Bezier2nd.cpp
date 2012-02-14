#include <cmath>
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{

Bezier2nd::Bezier2nd()
{
}

Bezier2nd::Bezier2nd(const Vector2& cp0_, const Vector2& cp1_, const Vector2& cp2_)
{
	cp0 = cp0_;
	cp1 = cp1_;
	cp2 = cp2_;
}

Vector2 Bezier2nd::evaluate(float t) const
{
	float it = 1.0f - t;
	return (it * it) * cp0 + (2.0f * it * t) * cp1 + (t * t) * cp2;
}

float Bezier2nd::getLocalMinMaxY() const
{
	return (cp0.y - cp1.y) / (cp0.y - 2.0f * cp1.y + cp2.y);
}

float Bezier2nd::getLocalMinMaxX() const
{
	return (cp0.x - cp1.x) / (cp0.x - 2.0f * cp1.x + cp2.x);
}

void Bezier2nd::intersectX(float y, float& outT0, float& outT1) const
{
	float a = cp0.y - 2.0f * cp1.y + cp2.y;
	if (abs< float >(a) > FUZZY_EPSILON)
	{
		float b = 2.0f * cp1.y - 2.0f * cp0.y;
		float c = cp0.y - y;

		float s = std::sqrt(b * b - 4.0f * a * c);
		float divisor = 2.0f * a;
		float divend0 = -b + s;
		float divend1 = -b - s;

		outT0 = divend0 / divisor;
		outT1 = divend1 / divisor;
	}
	else	// Not a 2nd degree polynomial
	{
		outT0 =
		outT1 = (y - cp0.y) / (2.0f * cp1.y - 2.0f *  cp0.y);
	}
}

void Bezier2nd::intersectY(float x, float& outT0, float& outT1) const
{
	float a = cp0.x - 2.0f * cp1.x + cp2.x;
	if (abs< float >(a) > FUZZY_EPSILON)
	{
		float b = 2.0f * cp1.x - 2.0f * cp0.x;
		float c = cp0.x - x;

		float s = std::sqrt(b * b - 4.0f * a * c);
		float divisor = 2.0f * a;
		float divend0 = -b + s;
		float divend1 = -b - s;

		outT0 = divend0 / divisor;
		outT1 = divend1 / divisor;
	}
	else	// Not a 2nd degree polynomial
	{
		outT0 =
		outT1 = (x - cp0.x) / (2.0f * cp1.x - 2.0f * cp0.x);
	}
}

void Bezier2nd::split(float t, Bezier2nd& outLeft, Bezier2nd& outRight) const
{
	Vector2 p = evaluate(t);
	outLeft = Bezier2nd(
		cp0,
		lerp(cp0, cp1, t),
		p
	);
	outRight = Bezier2nd(
		p,
		lerp(cp1, cp2, t),
		cp2
	);
}

}
