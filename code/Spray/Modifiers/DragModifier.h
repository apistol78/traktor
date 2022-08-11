#pragma once

#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! Drag modifier.
 * \ingroup Spray
 */
class DragModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	explicit DragModifier(float linearDrag, float angularDrag);

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const override final;

private:
	Scalar m_linearDrag;
	float m_angularDrag;
};

	}
}

