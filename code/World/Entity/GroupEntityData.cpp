#include <algorithm>
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/EntityInstance.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.world.GroupEntityData", GroupEntityData, EntityData)

void GroupEntityData::addInstance(EntityInstance* instance)
{
	m_instances.push_back(instance);
}

void GroupEntityData::removeInstance(EntityInstance* instance)
{
	RefArray< EntityInstance >::iterator i = std::find(m_instances.begin(), m_instances.end(), instance);
	if (i != m_instances.end())
		m_instances.erase(i);
}

void GroupEntityData::removeAllInstances()
{
	m_instances.resize(0);
}

RefArray< EntityInstance >& GroupEntityData::getInstances()
{
	return m_instances;
}

const RefArray< EntityInstance >& GroupEntityData::getInstances() const
{
	return m_instances;
}
	
bool GroupEntityData::serialize(Serializer& s)
{
	return s >> MemberRefArray< EntityInstance >(L"instances", m_instances);
}
	
	}
}
