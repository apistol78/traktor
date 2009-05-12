#include <algorithm>
#include "World/Entity/GroupEntityData.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.GroupEntityData", GroupEntityData, EntityData)

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

RefArray< EntityData >& GroupEntityData::getEntityData()
{
	return m_entityData;
}

const RefArray< EntityData >& GroupEntityData::getEntityData() const
{
	return m_entityData;
}
	
bool GroupEntityData::serialize(Serializer& s)
{
	if (!EntityData::serialize(s))
		return false;

	return s >> MemberRefArray< EntityData >(L"entityData", m_entityData);
}
	
	}
}
