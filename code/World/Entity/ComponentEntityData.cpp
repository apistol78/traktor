#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/IEntityComponentData.h"
#include "World/Entity/ComponentEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ComponentEntityData", 0, ComponentEntityData, EntityData)

void ComponentEntityData::setComponent(IEntityComponentData* component)
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

void ComponentEntityData::removeComponent(IEntityComponentData* component)
{
	auto it = std::find(m_components.begin(), m_components.end(), component);
	if (it != m_components.end())
		m_components.erase(it);
}

IEntityComponentData* ComponentEntityData::getComponent(const TypeInfo& componentType) const
{
	for (auto component : m_components)
	{
		if (is_type_a(componentType, type_of(component)))
			return component;
	}
	return nullptr;
}

const RefArray< IEntityComponentData >& ComponentEntityData::getComponents() const
{
	return m_components;
}

void ComponentEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);
	s >> MemberRefArray< IEntityComponentData >(L"components", m_components);
}

	}
}
