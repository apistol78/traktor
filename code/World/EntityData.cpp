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
	namespace
	{

class MemberEntityState : public MemberComplex
{
public:
	MemberEntityState(const wchar_t* const name, EntityState& ref, const Attribute& attributes)
	:	MemberComplex(name, true, attributes)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< bool >(L"visible", m_ref.visible);
		s >> Member< bool >(L"dynamic", m_ref.dynamic);
		s >> Member< bool >(L"locked", m_ref.locked);
	}

private:
	EntityState& m_ref;
};

	}

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

EntityState EntityData::setState(const EntityState& state, const EntityState& mask)
{
	const EntityState current = m_state;

	if (mask.visible)
		m_state.visible = state.visible;
	if (mask.dynamic)
		m_state.dynamic = state.dynamic;
	if (mask.locked)
		m_state.locked = state.locked;

	return current;
}

const EntityState& EntityData::getState() const
{
	return m_state;
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

	if (s.getVersion< EntityData >() >= 2)
		s >> MemberEntityState(L"state", m_state, AttributePrivate());

	s >> MemberComposite< Transform >(L"transform", m_transform);
	s >> MemberRefArray< IEntityComponentData >(L"components", m_components);
}

}
