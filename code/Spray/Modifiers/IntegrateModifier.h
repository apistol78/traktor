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
	explicit IntegrateModifier(float timeScale, bool linear, bool angular);

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const override final;

private:
	Scalar m_timeScale;
	bool m_linear;
	bool m_angular;
};

	}
}

