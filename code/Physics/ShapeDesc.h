#ifndef traktor_physics_ShapeDesc_H
#define traktor_physics_ShapeDesc_H

#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Transform.h"

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
class T_DLLCLASS ShapeDesc : public ISerializable
{
	T_RTTI_CLASS;

public:
	ShapeDesc();

	void setLocalTransform(const Transform& localTransform);

	const Transform& getLocalTransform() const;

	virtual bool serialize(ISerializer& s);

private:
	Transform m_localTransform;
};

	}
}

#endif	// traktor_physics_ShapeDesc_H
