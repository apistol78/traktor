#ifndef traktor_scene_EntityAdapterFactory_H
#define traktor_scene_EntityAdapterFactory_H

#include <map>
#include "Core/Heap/Ref.h"
#include "World/Entity/EntityFactory.h"

namespace traktor
{
	namespace scene
	{

class SceneEditorContext;
class EntityAdapter;

class EntityAdapterFactory : public world::EntityFactory
{
	T_RTTI_CLASS(EntityAdapterFactory)

public:
	EntityAdapterFactory(SceneEditorContext* context);

	void addFactory(world::EntityFactory* entityFactory);

	void beginBuild();

	EntityAdapter* endBuild();

	virtual const TypeSet getEntityTypes() const;

	virtual world::Entity* createEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const;

private:
	Ref< SceneEditorContext > m_context;
	RefArray< world::EntityFactory > m_entityFactories;
	mutable std::map< const world::EntityData*, RefList< EntityAdapter > > m_existingEntityAdapters;
	mutable Ref< EntityAdapter > m_rootEntityAdapter;
	mutable Ref< EntityAdapter > m_parentEntityAdapter;

	world::Entity* createRealEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const;
};

	}
}

#endif	// traktor_scene_EntityAdapterFactory_H
