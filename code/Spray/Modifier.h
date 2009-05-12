#ifndef traktor_spray_Modifier_H
#define traktor_spray_Modifier_H

#include "Core/Serialization/Serializable.h"
#include "Core/Math/Matrix44.h"
#include "Spray/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! \brief Emitter modifier.
 * \ingroup Spray
 */
class T_DLLCLASS Modifier : public Serializable
{
	T_RTTI_CLASS(Modifier)

public:
	virtual void update(const Scalar& deltaTime, const Matrix44& transform, PointVector& points, size_t first, size_t last) const = 0;
};

	}
}

#endif	// traktor_spray_Modifier_H
