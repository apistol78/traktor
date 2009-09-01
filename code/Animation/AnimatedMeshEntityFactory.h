#ifndef traktor_animation_AnimatedMeshEntityFactory_H
#define traktor_animation_AnimatedMeshEntityFactory_H

#include "Core/Heap/Ref.h"
#include "World/Entity/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
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

class PhysicsManager;

	}

	namespace animation
	{

/*! \brief Animated mesh entity factory.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS(AnimatedMeshEntityFactory)

public:
	AnimatedMeshEntityFactory(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager);

	virtual const TypeSet getEntityTypes() const;

	virtual world::Entity* createEntity(world::IEntityBuilder* builder, const std::wstring& name, const world::EntityData& entityData, const Object* instanceData) const;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< physics::PhysicsManager > m_physicsManager;
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntityFactory_H
