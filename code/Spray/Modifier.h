#pragma once

#include "Core/Math/Transform.h"
#include "Core/Object.h"
#include "Spray/Point.h"

namespace traktor
{

#if defined(T_MODIFIER_USE_PS3_SPURS)
class SpursJobQueue;
#endif

	namespace spray
	{

/*! Emitter modifier.
 * \ingroup Spray
 */
class Modifier : public Object
{
	T_RTTI_CLASS;

public:
	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const = 0;
};

	}
}

