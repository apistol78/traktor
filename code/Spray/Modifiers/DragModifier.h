#pragma once

#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! \brief Drag modifier.
 * \ingroup Spray
 */
class DragModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	DragModifier(float linearDrag, float angularDrag);

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const override final;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const override final;
#endif

private:
	Scalar m_linearDrag;
	float m_angularDrag;
};

	}
}

