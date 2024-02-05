/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupComponent", GroupComponent, IEntityComponent)

void GroupComponent::destroy()
{
	m_entities.resize(0);
}

void GroupComponent::setOwner(Entity* owner)
{
	T_ASSERT(m_owner == nullptr);
	if ((m_owner = owner) != nullptr)
		m_transform = m_owner->getTransform();
}

void GroupComponent::update(const UpdateParams& update)
{
}

void GroupComponent::setTransform(const Transform& transform)
{
	const Transform invTransform = m_transform.inverse();
	for (auto entity : m_entities)
	{
		const Transform currentTransform = entity->getTransform();
		const Transform Tlocal = invTransform * currentTransform;
		const Transform Tworld = transform * Tlocal;
		entity->setTransform(Tworld);
	}
	m_transform = transform;
}

void GroupComponent::setState(const EntityState& state, const EntityState& mask)
{
	for (auto entity : m_entities)
		entity->setState(state, mask);
}

Aabb3 GroupComponent::getBoundingBox() const
{
	const Transform invTransform = m_transform.inverse();

	Aabb3 boundingBox;
	for (auto entity : m_entities)
	{
		const Aabb3 childBoundingBox = entity->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			const Transform childTransform = entity->getTransform();
			const Transform intoParentTransform = invTransform * childTransform;
			boundingBox.contain(childBoundingBox.transform(intoParentTransform));
		}
	}

	return boundingBox;
}

void GroupComponent::addEntity(Entity* entity)
{
	T_ASSERT_M (entity, L"Null entity");
	m_entities.push_back(entity);
}

void GroupComponent::removeEntity(Entity* entity)
{
	T_ASSERT_M (entity, L"Null entity");
	m_entities.remove(entity);
}

void GroupComponent::removeAllEntities()
{
	m_entities.resize(0);
}

const RefArray< Entity >& GroupComponent::getEntities() const
{
	return m_entities;
}

Entity* GroupComponent::getEntity(const std::wstring& name, int32_t index) const
{
	for (auto entity : m_entities)
	{
		if (entity->getName() == name)
		{
			if (index-- <= 0)
				return entity;
		}
	}
	return nullptr;
}

RefArray< Entity > GroupComponent::getEntities(const std::wstring& name) const
{
	RefArray< Entity > entities;
	for (auto entity : m_entities)
	{
		if (entity->getName() == name)
			entities.push_back(entity);
	}
	return entities;
}

}
