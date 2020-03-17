#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/EntityData.h"
#include "World/IEntityComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.EntityData", 0, EntityData, ISerializable)

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

void EntityData::removeComponent(IEntityComponentData* component)
{
	auto it = std::find(m_components.begin(), m_components.end(), component);
	if (it != m_components.end())
		m_components.erase(it);
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

const RefArray< IEntityComponentData >& EntityData::getComponents() const
{
	return m_components;
}

void EntityData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberComposite< Transform >(L"transform", m_transform);
	s >> MemberRefArray< IEntityComponentData >(L"components", m_components);
}

	}
}
