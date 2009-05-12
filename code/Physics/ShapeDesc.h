#ifndef traktor_physics_ShapeDesc_H
#define traktor_physics_ShapeDesc_H

#include "Core/Serialization/Serializable.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Collision shape description.
 * \ingroup Physics
 */
class T_DLLCLASS ShapeDesc : public Serializable
{
	T_RTTI_CLASS(ShapeDesc)

public:
	ShapeDesc();

	void setLocalTransform(const Matrix44& localTransform);

	const Matrix44& getLocalTransform() const;

	virtual bool serialize(Serializer& s);

private:
	Matrix44 m_localTransform;
};

	}
}

#endif	// traktor_physics_ShapeDesc_H
