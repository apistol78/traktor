#ifndef traktor_terrain_EntityFactory_H
#define traktor_terrain_EntityFactory_H

#include "Core/Heap/Ref.h"
#include "World/Entity/EntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderSystem;

	}

	namespace terrain
	{

class T_DLLCLASS EntityFactory : public world::EntityFactory
{
	T_RTTI_CLASS(EntityFactory)

public:
	EntityFactory(render::RenderSystem* renderSystem);

	virtual const TypeSet getEntityTypes() const;

	virtual world::Entity* createEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const;

private:
	Ref< render::RenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_terrain_EntityFactory_H
