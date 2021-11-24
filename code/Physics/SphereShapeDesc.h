#pragma once

#include "Physics/ShapeDesc.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! Sphere collision shape.
 * \ingroup Physics
 */
class T_DLLCLASS SphereShapeDesc : public ShapeDesc
{
	T_RTTI_CLASS;

public:
	SphereShapeDesc();

	void setRadius(float radius);

	float getRadius() const;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_radius;
};

	}
}

