/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/Entity/FacadeComponent.h"
#include "World/Entity/ScriptComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.FacadeComponent", FacadeComponent, IEntityComponent)

FacadeComponent::FacadeComponent()
:	m_owner(nullptr)
{
}

void FacadeComponent::destroy()
{
	for (auto it : m_entities)
	{
		if (it.second)
			it.second->destroy();
	}
	m_entities.clear();
	m_visibleEntities.clear();
}

void FacadeComponent::setOwner(Entity* owner)
{
	T_ASSERT(m_owner == nullptr);
	if ((m_owner = owner) != nullptr)
		m_transform = m_owner->getTransform();
}

void FacadeComponent::update(const UpdateParams& update)
{
	for (auto entity : m_visibleEntities)
		entity->update(update);
}

void FacadeComponent::setTransform(const Transform& transform)
{
	Transform invTransform = m_transform.inverse();
	for (auto it : m_entities)
	{
		Transform currentTransform = it.second->getTransform();
		Transform Tlocal = invTransform * currentTransform;
		Transform Tworld = transform * Tlocal;
		it.second->setTransform(Tworld);
	}
	m_transform = transform;
}

Aabb3 FacadeComponent::getBoundingBox() const
{
	Transform invTransform = m_transform.inverse();

	Aabb3 boundingBox;
	for (auto entity : m_visibleEntities)
	{
		Aabb3 childBoundingBox = entity->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			Transform childTransform = entity->getTransform();
			Transform intoParentTransform = invTransform * childTransform;
			boundingBox.contain(childBoundingBox.transform(intoParentTransform));
		}
	}

	return boundingBox;
}

void FacadeComponent::addEntity(const std::wstring& id, Entity* entity)
{
	m_entities.insert(id, entity);
}

void FacadeComponent::removeEntity(const std::wstring& id)
{
	m_entities.remove(id);
}

bool FacadeComponent::show(const std::wstring& id)
{
	auto it = m_entities.find(id);
	if (it == m_entities.end())
		return false;

	if (m_visibleEntities.insert(it->second))
	{
		auto script = it->second->getComponent< ScriptComponent >();
		if (script)
			script->execute("show");
	}

	return true;
}

bool FacadeComponent::showOnly(const std::wstring& id)
{
	hideAll();
	return show(id);
}

bool FacadeComponent::hide(const std::wstring& id)
{
	auto it = m_entities.find(id);
	if (it == m_entities.end())
		return false;

	if (m_visibleEntities.erase(it->second))
	{
		auto script = it->second->getComponent< ScriptComponent >();
		if (script)
			script->execute("hide");
	}

	return true;
}

void FacadeComponent::hideAll()
{
	m_visibleEntities.clear();
}

bool FacadeComponent::isVisible(const std::wstring& id)
{
	auto it = m_entities.find(id);
	if (it == m_entities.end())
		return false;
	return m_visibleEntities.find(it->second) != m_visibleEntities.end();
}

	}
}
