#pragma once

#include "Core/Math/Random.h"
#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! Brownian motion modifier.
 * \ingroup Spray
 */
class BrownianModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	explicit BrownianModifier(float factor);

	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const override final;

private:
	Scalar m_factor;
	mutable Random m_random;
};

	}
}

