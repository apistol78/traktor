#ifndef traktor_terrain_EntityFactory_H
#define traktor_terrain_EntityFactory_H

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
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

	namespace render
	{

class IRenderSystem;

	}

	namespace terrain
	{

/*! \brief Terrain entity factory.
 * \ingroup Terrain
 */
class T_DLLCLASS EntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	EntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual const TypeInfoSet getEntityEventTypes() const;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const;

private:
	resource::IResourceManager* m_resourceManager;
	render::IRenderSystem* m_renderSystem;
};

	}
}

#endif	// traktor_terrain_EntityFactory_H
