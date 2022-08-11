#pragma once

#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! Particle size modifier.
 * \ingroup Spray
 */
class SizeModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	explicit SizeModifier(float adjustRate);

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const override final;

private:
	float m_adjustRate;
};

	}
}

