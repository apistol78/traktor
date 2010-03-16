#ifndef traktor_spray_Modifier_H
#define traktor_spray_Modifier_H

#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"
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

#if defined(_PS3)
class SpursJobQueue;
#endif

	namespace spray
	{

/*! \brief Emitter modifier.
 * \ingroup Spray
 */
class T_DLLCLASS Modifier : public ISerializable
{
	T_RTTI_CLASS;

public:
#if defined(_PS3)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const = 0;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const = 0;
#endif
};

	}
}

#endif	// traktor_spray_Modifier_H
