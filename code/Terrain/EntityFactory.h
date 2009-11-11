#ifndef traktor_terrain_EntityFactory_H
#define traktor_terrain_EntityFactory_H

#include "Core/Heap/Ref.h"
#include "World/Entity/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
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

	namespace render
	{

class IRenderSystem;

	}

	namespace terrain
	{

class T_DLLCLASS EntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS(EntityFactory)

public:
	EntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	virtual const TypeSet getEntityTypes() const;

	virtual Ref< world::Entity > createEntity(world::IEntityBuilder* builder, const std::wstring& name, const world::EntityData& entityData, const Object* instanceData) const;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_terrain_EntityFactory_H
