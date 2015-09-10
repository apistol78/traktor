#ifndef traktor_Bezier3rd_H
#define traktor_Bezier3rd_H

#include "Core/Containers/AlignedVector.h"
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

class Bezier2nd;

/*! \brief Cubic bezier curve.
 * \ingroup Core
 */
class T_DLLCLASS Bezier3rd
{
public:
	Vector2 cp0;
	Vector2 cp1;
	Vector2 cp2;
	Vector2 cp3;

	Bezier3rd();

	Bezier3rd(const Vector2& cp0, const Vector2& cp1, const Vector2& cp2, const Vector2& cp3);

	Vector2 evaluate(float t) const;

	Vector2 tangent(float t) const;

	void split(float t, Bezier3rd& outLeft, Bezier3rd& outRight) const;

	void approximate(AlignedVector< Bezier2nd >& outQuadratic) const;
};

}

#endif	// traktor_Bezier3rd_H
