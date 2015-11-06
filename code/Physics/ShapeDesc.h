#ifndef traktor_physics_ShapeDesc_H
#define traktor_physics_ShapeDesc_H

#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"

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

	void setCollisionGroup(uint32_t collisionGroup);

	uint32_t getCollisionGroup() const;

	void setCollisionMask(uint32_t collisionMask);

	uint32_t getCollisionMask() const;

	void setMaterial(int32_t material);

	int32_t getMaterial() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE;

private:
	Transform m_localTransform;
	uint32_t m_collisionGroup;
	uint32_t m_collisionMask;
	int32_t m_material;
};

	}
}

#endif	// traktor_physics_ShapeDesc_H
