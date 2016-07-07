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

	virtual const TypeInfoSet getEntityTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getEntityEventTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getEntityComponentTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const T_OVERRIDE T_FINAL;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const T_OVERRIDE T_FINAL;

	virtual Ref< world::IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const T_OVERRIDE T_FINAL;

private:
	resource::IResourceManager* m_resourceManager;
	physics::PhysicsManager* m_physicsManager;
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntityFactory_H
