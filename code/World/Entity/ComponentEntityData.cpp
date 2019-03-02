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
	for (RefArray< IEntityComponentData >::iterator i = m_components.begin(); i != m_components.end(); ++i)
	{
		if (is_type_a(type_of(*i), type_of(component)))
		{
			*i = component;
			return;
		}
	}
	m_components.push_back(component);
}

IEntityComponentData* ComponentEntityData::getComponent(const TypeInfo& componentType) const
{
	for (RefArray< IEntityComponentData >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
	{
		if (is_type_a(componentType, type_of(*i)))
			return *i;
	}
	return 0;
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
