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
class IEntityEditorFactory;
class SceneEditorContext;

class EntityAdapterBuilder : public world::IEntityBuilder
{
	T_RTTI_CLASS;

public:
	EntityAdapterBuilder(
		SceneEditorContext* context,
		world::IEntityBuilder* entityBuilder,
		const RefArray< const IEntityEditorFactory >& entityEditorFactories
	);

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
	Ref< SceneEditorContext > m_context;
	Ref< world::IEntityBuilder > m_entityBuilder;
	RefArray< const IEntityEditorFactory > m_entityEditorFactories;
	mutable std::map< const TypeInfo*, RefArray< EntityAdapter > > m_cachedAdapters;
	mutable Ref< EntityAdapter > m_currentAdapter;
	mutable Ref< EntityAdapter > m_rootAdapter;
	mutable uint32_t m_adapterCount;
};

	}
}

#endif	// traktor_scene_EntityAdapterBuilder_H
