#pragma once

#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! \brief Gravity modifier.
 * \ingroup Spray
 */
class GravityModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	GravityModifier(const Vector4& gravity, bool world);

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const override final;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const override final;
#endif

private:
	Vector4 m_gravity;
	bool m_world;
};

	}
}

