#ifndef traktor_scene_EntityAdapterBuilder_H
#define traktor_scene_EntityAdapterBuilder_H

#include <map>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Timer/Timer.h"
#include "World/IEntityBuilder.h"

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
	EntityAdapterBuilder(
		SceneEditorContext* context,
		world::IEntityBuilder* entityBuilder,
		const RefArray< const IEntityEditorFactory >& entityEditorFactories,
		EntityAdapter* currentEntityAdapter
	);

	virtual ~EntityAdapterBuilder();

	virtual void addFactory(const world::IEntityFactory* entityFactory);

	virtual void removeFactory(const world::IEntityFactory* entityFactory);

	virtual const world::IEntityFactory* getFactory(const world::EntityData* entityData) const;

	virtual const world::IEntityFactory* getFactory(const world::IEntityEventData* entityEventData) const;

	virtual Ref< world::Entity > create(const world::EntityData* entityData) const;

	virtual Ref< world::IEntityEvent > create(const world::IEntityEventData* entityEventData) const;

	virtual const world::IEntityBuilder* getCompositeEntityBuilder() const;

	EntityAdapter* getRootAdapter() const;

	uint32_t getAdapterCount() const;

private:
	struct Cache
	{
		RefArray< EntityAdapter > adapters;
		std::map< uint32_t, RefArray< world::Entity > > leafEntities;
	};

	Ref< SceneEditorContext > m_context;
	Ref< world::IEntityBuilder > m_entityBuilder;
	RefArray< const IEntityEditorFactory > m_entityEditorFactories;
	mutable std::map< const TypeInfo*, Cache > m_cache;
	mutable Ref< EntityAdapter > m_currentAdapter;
	mutable Ref< EntityAdapter > m_rootAdapter;
	mutable uint32_t m_adapterCount;
	mutable Timer m_timer;
	mutable std::vector< double > m_buildTimeStack;
	mutable std::map< const TypeInfo*, std::pair< int32_t, double > > m_buildTimes;
	mutable std::set< const world::Entity* > m_builtEntities;
};

	}
}

#endif	// traktor_scene_EntityAdapterBuilder_H
