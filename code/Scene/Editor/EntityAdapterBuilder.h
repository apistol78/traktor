#ifndef traktor_scene_EntityAdapterBuilder_H
#define traktor_scene_EntityAdapterBuilder_H

#include <map>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace scene
	{

class EntityAdapter;
class SceneEditorContext;

class EntityAdapterBuilder : public world::IEntityBuilder
{
	T_RTTI_CLASS;

public:
	EntityAdapterBuilder(
		SceneEditorContext* context,
		world::IEntityBuilder* entityBuilder,
		EntityAdapter* currentEntityAdapter
	);

	virtual ~EntityAdapterBuilder();

	virtual void addFactory(const world::IEntityFactory* entityFactory) T_OVERRIDE T_FINAL;

	virtual void removeFactory(const world::IEntityFactory* entityFactory) T_OVERRIDE T_FINAL;

	virtual const world::IEntityFactory* getFactory(const world::EntityData* entityData) const T_OVERRIDE T_FINAL;

	virtual const world::IEntityFactory* getFactory(const world::IEntityEventData* entityEventData) const T_OVERRIDE T_FINAL;

	virtual const world::IEntityFactory* getFactory(const world::IEntityComponentData* entityComponentData) const T_OVERRIDE T_FINAL;

	virtual Ref< world::Entity > create(const world::EntityData* entityData) const T_OVERRIDE T_FINAL;

	virtual Ref< world::IEntityEvent > create(const world::IEntityEventData* entityEventData) const T_OVERRIDE T_FINAL;

	virtual Ref< world::IEntityComponent > create(const world::IEntityComponentData* entityComponentData) const T_OVERRIDE T_FINAL;

	virtual const world::IEntityBuilder* getCompositeEntityBuilder() const T_OVERRIDE T_FINAL;

	EntityAdapter* getRootAdapter() const;

private:
	struct Cache
	{
		RefArray< EntityAdapter > adapters;
		std::map< uint32_t, RefArray< world::Entity > > leafEntities;
	};

	SceneEditorContext* m_context;
	Ref< world::IEntityBuilder > m_entityBuilder;
	mutable std::map< const TypeInfo*, Cache > m_cache;
	mutable Ref< EntityAdapter > m_currentAdapter;
	mutable Ref< EntityAdapter > m_rootAdapter;
};

	}
}

#endif	// traktor_scene_EntityAdapterBuilder_H
