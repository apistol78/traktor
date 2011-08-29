#ifndef traktor_scene_EntityAdapterBuilder_H
#define traktor_scene_EntityAdapterBuilder_H

#include <map>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "World/Entity/IEntityBuilder.h"

namespace traktor
{
	namespace scene
	{

class EntityAdapter;
class IEntityEditorFactory;
class SceneEditorContext;

class EntityAdapterBuilder : public world::IEntityBuilder
{
	T_RTTI_CLASS;

public:
	EntityAdapterBuilder(SceneEditorContext* context, const RefArray< IEntityEditorFactory >& entityEditorFactories);

	virtual void addFactory(world::IEntityFactory* entityFactory);

	virtual void removeFactory(world::IEntityFactory* entityFactory);

	virtual void begin(world::IEntitySchema* entitySchema);

	virtual Ref< world::Entity > create(const world::EntityData* entityData);

	virtual Ref< world::Entity > get(const world::EntityData* entityData) const;

	virtual void end();

	Ref< EntityAdapter > getRootAdapter() const;

private:
	Ref< SceneEditorContext > m_context;
	RefArray< IEntityEditorFactory > m_entityEditorFactories;
	Ref< world::IEntitySchema > m_entitySchema;
	RefArray< world::IEntityFactory > m_entityFactories;
	std::map< const world::EntityData*, Ref< world::Entity > > m_entities;
	std::map< const world::EntityData*, RefArray< EntityAdapter > > m_cachedAdapters;
	Ref< EntityAdapter > m_currentAdapter;
	Ref< EntityAdapter > m_rootAdapter;
};

	}
}

#endif	// traktor_scene_EntityAdapterBuilder_H
