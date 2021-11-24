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

/*! Capsule collision shape.
 * \ingroup Physics
 */
class T_DLLCLASS CapsuleShapeDesc : public ShapeDesc
{
	T_RTTI_CLASS;

public:
	CapsuleShapeDesc();

	void setRadius(float radius);

	float getRadius() const;

	void setLength(float length);

	float getLength() const;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_radius;
	float m_length;
};

	}
}

