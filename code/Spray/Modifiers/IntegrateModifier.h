#pragma once

#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! Integrate particle velocity modifier.
 * \ingroup Spray
 */
class IntegrateModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	IntegrateModifier(float timeScale, bool linear, bool angular);

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const override final;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const override final;
#endif

private:
	Scalar m_timeScale;
	bool m_linear;
	bool m_angular;
};

	}
}

