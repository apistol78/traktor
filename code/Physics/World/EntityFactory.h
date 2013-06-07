#ifndef traktor_physics_EntityFactory_H
#define traktor_physics_EntityFactory_H

#include "World/IEntityFactory.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
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

	namespace world
	{

class IEntityEventManager;

	}

	namespace physics
	{

class PhysicsManager;

/*! \brief Physics entity factory.
 * \ingroup Physics
 */
class T_DLLCLASS EntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	EntityFactory(
		world::IEntityEventManager* eventManager,
		resource::IResourceManager* resourceManager,
		PhysicsManager* physicsManager
	);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual const TypeInfoSet getEntityEventTypes() const;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const;

private:
	Ref< world::IEntityEventManager > m_eventManager;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< PhysicsManager > m_physicsManager;
};

	}
}

#endif	// traktor_physics_EntityFactory_H
