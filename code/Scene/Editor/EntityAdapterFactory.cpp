#include "Scene/Editor/EntityAdapterFactory.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity/EntityFactory.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/Entity.h"
#include "Core/Misc/Save.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapterFactory", EntityAdapterFactory, world::EntityFactory)

EntityAdapterFactory::EntityAdapterFactory(SceneEditorContext* context)
:	m_context(context)
{
}

void EntityAdapterFactory::addFactory(world::EntityFactory* entityFactory)
{
	m_entityFactories.push_back(entityFactory);
}

void EntityAdapterFactory::beginBuild()
{
	m_rootEntityAdapter = 0;

	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		EntityAdapter* entityAdapter = *i;

		// Create map from existing entity data to adapters.
		m_existingEntityAdapters[entityAdapter->getEntityData()].push_back(*i);

		// Unlink adapter from hierarchy.
		if (entityAdapter->getParent())
			entityAdapter->removeFromParent();
		entityAdapter->removeAllChildren();

		// Nuke attached entity.
		if (entityAdapter->getEntity())
		{
			entityAdapter->getEntity()->destroy();
			entityAdapter->setEntity(0);
		}
	}
}

EntityAdapter* EntityAdapterFactory::endBuild()
{
	m_existingEntityAdapters.clear();
	return m_rootEntityAdapter;
}

const TypeSet EntityAdapterFactory::getEntityTypes() const
{
	TypeSet typeSet;
	for (RefArray< world::EntityFactory >::const_iterator i = m_entityFactories.begin(); i != m_entityFactories.end(); ++i)
	{
		TypeSet realTypeSet = (*i)->getEntityTypes();
		typeSet.insert(realTypeSet.begin(), realTypeSet.end());
	}
	return typeSet;
}

world::Entity* EntityAdapterFactory::createEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const
{
	Ref< EntityAdapter > entityAdapter;
	Ref< world::Entity > entity;

	// Do we already have an existing adapter for this type of entity?
	std::map< const world::EntityData*, RefList< EntityAdapter > >::iterator i = m_existingEntityAdapters.find(&entityData);
	if (i != m_existingEntityAdapters.end() && !i->second.empty())
	{
		// Pop adapter from list of adapters of same type.
		entityAdapter = i->second.front(); i->second.pop_front();
		T_ASSERT (entityAdapter);
	}
	else
	{
		// Create new entity adapter.
		entityAdapter = gc_new< EntityAdapter >(const_cast< world::EntityData* >(&entityData));
	}

	// Insert into hierarchy.
	if (m_parentEntityAdapter)
	{
		entityAdapter->setParent(m_parentEntityAdapter);
		m_parentEntityAdapter->addChild(entityAdapter, false);
	}
	else
		m_rootEntityAdapter = entityAdapter;

	// Create entity from original entity factories.
	{
		Save< Ref< EntityAdapter > > __save__(m_parentEntityAdapter, entityAdapter);
		entity = createRealEntity(builder, entityData);
	}

	// Save created entity in adapter, note: there might not be an entity
	// created as new entity data might be invalid but we must have the
	// adapter ready anyway so we can modify the data later.
	entityAdapter->setEntity(entity);
	entityAdapter->resetModified();

	return entity;
}

world::Entity* EntityAdapterFactory::createRealEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const
{
	for (RefArray< world::EntityFactory >::const_iterator i = m_entityFactories.begin(); i != m_entityFactories.end(); ++i)
	{
		TypeSet typeSet = (*i)->getEntityTypes();
		for (TypeSet::const_iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			if (is_type_of(**j, entityData.getType()))
				return (*i)->createEntity(builder, entityData);
		}
	}
	return 0;
}

	}
}
