#ifndef traktor_Bezier2nd_H
#define traktor_Bezier2nd_H

#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Bezier3rd;

/*! \brief Quadratic bezier curve.
 * \ingroup Core
 */
class T_DLLCLASS Bezier2nd
{
public:
	Vector2 cp0;
	Vector2 cp1;
	Vector2 cp2;

	Bezier2nd();

	Bezier2nd(const Vector2& cp0, const Vector2& cp1, const Vector2& cp2);

	Vector2 evaluate(float t) const;

	Vector2 tangent(float t) const;

	float getLocalMinMaxY() const;

	float getLocalMinMaxX() const;

	void intersectX(float y, float& outT0, float& outT1) const;

	void intersectY(float x, float& outT0, float& outT1) const;

	void split(float t, Bezier2nd& outLeft, Bezier2nd& outRight) const;

	void toBezier3rd(Bezier3rd& out3rd) const;

	/*! \brief Construct a 2nd order bezier curve from three points
	 * by which all three lies on the curve.
	 */
	static Bezier2nd fromPoints(const Vector2& p0, const Vector2& p1, const Vector2& p2);
};

}

#endif	// traktor_Bezier2nd_H
