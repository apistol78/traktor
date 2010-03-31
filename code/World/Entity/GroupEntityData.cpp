#include <algorithm>
#include "World/Entity/GroupEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.GroupEntityData", 0, GroupEntityData, EntityData)

void GroupEntityData::addEntityData(EntityData* entityData)
{
	m_entityData.push_back(entityData);
}

void GroupEntityData::removeEntityData(EntityData* entityData)
{
	RefArray< EntityData >::iterator i = std::find(m_entityData.begin(), m_entityData.end(), entityData);
	if (i != m_entityData.end())
		m_entityData.erase(i);
}

void GroupEntityData::removeAllEntityData()
{
	m_entityData.resize(0);
}

RefArray< EntityData >& GroupEntityData::getEntityData()
{
	return m_entityData;
}

const RefArray< EntityData >& GroupEntityData::getEntityData() const
{
	return m_entityData;
}
	
bool GroupEntityData::serialize(ISerializer& s)
{
	if (!EntityData::serialize(s))
		return false;

	return s >> MemberRefArray< EntityData >(L"entityData", m_entityData);
}
	
	}
}
