/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/Entity/FacadeComponent.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.FacadeComponent", FacadeComponent, IEntityComponent)

void FacadeComponent::destroy()
{
}

void FacadeComponent::setOwner(Entity* owner)
{
	m_owner = owner;
}

void FacadeComponent::update(const UpdateParams& update)
{
}

void FacadeComponent::setTransform(const Transform& transform)
{
}

Aabb3 FacadeComponent::getBoundingBox() const
{
	return Aabb3();
}

bool FacadeComponent::show(const std::wstring& id)
{
	auto group = m_owner->getComponent< GroupComponent >();
	if (!group)
		return false;
	
	Entity* showEntity = group->getEntity(id, 0);
	if (!showEntity)
		return false;

	showEntity->modifyState(Entity::Visible, 0);
	return true;
}

bool FacadeComponent::showOnly(const std::wstring& id)
{
	hideAll();
	return show(id);
}

bool FacadeComponent::hide(const std::wstring& id)
{
	auto group = m_owner->getComponent< GroupComponent >();
	if (!group)
		return false;
	
	Entity* hideEntity = group->getEntity(id, 0);
	if (!hideEntity)
		return false;

	hideEntity->modifyState(0, Entity::Visible);
	return true;
}

void FacadeComponent::hideAll()
{
	auto group = m_owner->getComponent< GroupComponent >();
	if (!group)
		return;

	for (auto entity : group->getEntities())
		entity->modifyState(0, Entity::Visible);
}

bool FacadeComponent::isVisible(const std::wstring& id)
{
	auto group = m_owner->getComponent< GroupComponent >();
	if (!group)
		return false;
	
	const Entity* entity = group->getEntity(id, 0);
	if (!entity)
		return false;

	return entity->isVisible();
}

}
