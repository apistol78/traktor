/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "World/EntityBuilderWithSchema.h"
#include "World/EntityData.h"
#include "World/IEntityFactory.h"
#include "World/IEntitySchema.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityBuilderWithSchema", EntityBuilderWithSchema, IEntityBuilder)

EntityBuilderWithSchema::EntityBuilderWithSchema(IEntityBuilder* entityBuilder, IEntitySchema* entitySchema)
:	m_entityBuilder(entityBuilder)
,	m_entitySchema(entitySchema)
,	m_outEntityProducts(0)
{
}

EntityBuilderWithSchema::EntityBuilderWithSchema(IEntityBuilder* entityBuilder, IEntitySchema* entitySchema, std::map< const world::EntityData*, Ref< world::Entity > >& outEntityProducts)
:	m_entityBuilder(entityBuilder)
,	m_entitySchema(entitySchema)
,	m_outEntityProducts(&outEntityProducts)
{
}

void EntityBuilderWithSchema::addFactory(const IEntityFactory* entityFactory)
{
	m_entityBuilder->addFactory(entityFactory);
}

void EntityBuilderWithSchema::removeFactory(const IEntityFactory* entityFactory)
{
	m_entityBuilder->removeFactory(entityFactory);
}

const IEntityFactory* EntityBuilderWithSchema::getFactory(const EntityData* entityData) const
{
	return m_entityBuilder->getFactory(entityData);
}

const IEntityFactory* EntityBuilderWithSchema::getFactory(const IEntityEventData* entityEventData) const
{
	return m_entityBuilder->getFactory(entityEventData);
}

const IEntityFactory* EntityBuilderWithSchema::getFactory(const IEntityComponentData* entityComponentData) const
{
	return m_entityBuilder->getFactory(entityComponentData);
}

Ref< Entity > EntityBuilderWithSchema::create(const EntityData* entityData) const
{
	Ref< const IEntityFactory > entityFactory = m_entityBuilder->getFactory(entityData);
	if (!entityFactory)
		return 0;

	m_entityScope.push(scope_t());

	Ref< Entity > entity = entityFactory->createEntity(this, *entityData);

	const scope_t& scope = m_entityScope.top();
	for (scope_t::const_iterator i = scope.begin(); i != scope.end(); ++i)
	{
		if (i->second != entity)
			m_entitySchema->insertEntity(entity, i->first, i->second);
	}

	m_entityScope.pop();

	if (!m_entityScope.empty())
	{
		scope_t& scopeTop = m_entityScope.top();
		scopeTop.push_back(std::make_pair(entityData->getName(), entity));
	}
	else
		m_entitySchema->insertEntity(0, entityData->getName(), entity);

	if (m_outEntityProducts)
		(*m_outEntityProducts)[entityData] = entity;

	return entity;
}

Ref< IEntityEvent > EntityBuilderWithSchema::create(const IEntityEventData* entityEventData) const
{
	return m_entityBuilder->create(entityEventData);
}

Ref< IEntityComponent > EntityBuilderWithSchema::create(const IEntityComponentData* entityComponentData) const
{
	return m_entityBuilder->create(entityComponentData);
}

const IEntityBuilder* EntityBuilderWithSchema::getCompositeEntityBuilder() const
{
	return m_entityBuilder->getCompositeEntityBuilder();
}

	}
}
