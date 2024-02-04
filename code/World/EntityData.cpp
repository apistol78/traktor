/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberBitMask.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/EntityData.h"
#include "World/IEntityComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.EntityData", 2, EntityData, ISerializable)

void EntityData::setId(const Guid& id)
{
	m_id = id;
}

const Guid& EntityData::getId() const
{
	return m_id;
}

void EntityData::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& EntityData::getName() const
{
	return m_name;
}

void EntityData::setTransform(const Transform& transform)
{
	for (auto component : m_components)
		component->setTransform(this, transform);
	m_transform = transform;
}

const Transform& EntityData::getTransform() const
{
	return m_transform;
}

void EntityData::setState(EntityState state)
{
	m_state = state;
}

EntityState EntityData::getState() const
{
	return m_state;
}

void EntityData::setComponent(IEntityComponentData* component)
{
	const auto& componentType = type_of(component);
	for (auto existingComponent : m_components)
	{
		if (is_type_a(type_of(existingComponent), componentType))
		{
			existingComponent = component;
			return;
		}
	}
	m_components.push_back(component);
}

bool EntityData::removeComponent(const IEntityComponentData* component)
{
	auto it = std::find(m_components.begin(), m_components.end(), component);
	if (it != m_components.end())
	{
		m_components.erase(it);
		return true;
	}
	else
		return false;
}

IEntityComponentData* EntityData::getComponent(const TypeInfo& componentType) const
{
	for (auto component : m_components)
	{
		if (is_type_a(componentType, type_of(component)))
			return component;
	}
	return nullptr;
}

void EntityData::setComponents(const RefArray< IEntityComponentData >& components)
{
	m_components = components;
}

const RefArray< IEntityComponentData >& EntityData::getComponents() const
{
	return m_components;
}

void EntityData::serialize(ISerializer& s)
{
	if (s.getVersion< EntityData >() >= 1)
		s >> Member< Guid >(L"id", m_id, AttributePrivate());

	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberComposite< Transform >(L"transform", m_transform);

	if (s.getVersion< EntityData >() >= 2)
	{
		const MemberBitMask::Bit c_EntityState_bits[] =
		{
			{ L"visible", (uint32_t)EntityState::Visible },
			{ L"dynamic", (uint32_t)EntityState::Dynamic },
			{ L"locked", (uint32_t)EntityState::Locked },
			{ 0 }
		};
		s >> MemberBitMask(L"state", (uint32_t&)m_state, c_EntityState_bits, AttributePrivate());
	}

	s >> MemberRefArray< IEntityComponentData >(L"components", m_components);
}

}
