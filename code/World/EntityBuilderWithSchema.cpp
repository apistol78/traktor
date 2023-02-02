/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/EntityBuilderWithSchema.h"
#include "World/EntityData.h"
#include "World/IEntityFactory.h"
#include "World/EntitySchema.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityBuilderWithSchema", EntityBuilderWithSchema, IEntityBuilder)

EntityBuilderWithSchema::EntityBuilderWithSchema(IEntityBuilder* entityBuilder, EntitySchema* entitySchema)
:	m_entityBuilder(entityBuilder)
,	m_entitySchema(entitySchema)
,	m_outEntityProducts(nullptr)
{
}

EntityBuilderWithSchema::EntityBuilderWithSchema(IEntityBuilder* entityBuilder, EntitySchema* entitySchema, SmallMap< Guid, Ref< world::Entity > >& outEntityProducts)
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
	const IEntityFactory* entityFactory = m_entityBuilder->getFactory(entityData);
	if (!entityFactory)
		return nullptr;

	m_entityScope.push(scope_t());

	Ref< Entity > entity = entityFactory->createEntity(this, *entityData);

	if (m_entitySchema)
	{
		const scope_t& scope = m_entityScope.top();
		for (scope_t::const_iterator i = scope.begin(); i != scope.end(); ++i)
		{
			if (i->second != entity)
				m_entitySchema->insertEntity(entity, i->first, i->second);
		}
	}

	m_entityScope.pop();

	if (!m_entityScope.empty())
	{
		scope_t& scopeTop = m_entityScope.top();
		scopeTop.push_back(std::make_pair(entityData->getName(), entity));
	}
	else if (m_entitySchema)
		m_entitySchema->insertEntity(nullptr, entityData->getName(), entity);

	if (m_outEntityProducts && entityData->getId().isNotNull())
		(*m_outEntityProducts)[entityData->getId()] = entity;

	return entity;
}

Ref< IEntityEvent > EntityBuilderWithSchema::create(const IEntityEventData* entityEventData) const
{
	const IEntityFactory* entityFactory = m_entityBuilder->getFactory(entityEventData);
	if (entityFactory)
		return entityFactory->createEntityEvent(this, *entityEventData);
	else
		return nullptr;
}

Ref< IEntityComponent > EntityBuilderWithSchema::create(const IEntityComponentData* entityComponentData) const
{
	const IEntityFactory* entityFactory = m_entityBuilder->getFactory(entityComponentData);
	if (entityFactory)
		return entityFactory->createEntityComponent(this, *entityComponentData);
	else
		return nullptr;
}

const IEntityBuilder* EntityBuilderWithSchema::getCompositeEntityBuilder() const
{
	return m_entityBuilder->getCompositeEntityBuilder();
}

}
