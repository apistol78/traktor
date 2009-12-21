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
	namespace resource
	{

class IResourceManager;

	}

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

	void setGroup(uint32_t group);

	uint32_t getGroup() const;

	virtual bool bind(resource::IResourceManager* resourceManager);

	virtual bool serialize(ISerializer& s);

private:
	Transform m_localTransform;
	uint32_t m_group;
};

	}
}

#endif	// traktor_physics_ShapeDesc_H
