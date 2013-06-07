#ifndef traktor_animation_AnimatedMeshEntityFactory_H
#define traktor_animation_AnimatedMeshEntityFactory_H

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS;

public:
	AnimatedMeshEntityFactory(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual const TypeInfoSet getEntityEventTypes() const;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const;

private:
	resource::IResourceManager* m_resourceManager;
	physics::PhysicsManager* m_physicsManager;
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntityFactory_H
