#ifndef traktor_spray_Modifier_H
#define traktor_spray_Modifier_H

#if defined(_PS3)
#	define T_MODIFIER_USE_PS3_SPURS
#endif

#include "Core/Math/Transform.h"
#include "Core/Object.h"
#include "Spray/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

#if defined(T_MODIFIER_USE_PS3_SPURS)
class SpursJobQueue;
#endif

	namespace spray
	{

/*! \brief Emitter modifier.
 * \ingroup Spray
 */
class T_DLLCLASS Modifier : public Object
{
	T_RTTI_CLASS;

public:
#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const = 0;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const = 0;
#endif
};

	}
}

#endif	// traktor_spray_Modifier_H
