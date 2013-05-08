#include <algorithm>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/Entity/GroupEntityData.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.GroupEntityData", 0, GroupEntityData, EntityData)

void GroupEntityData::addEntityData(EntityData* entityData)
{
	T_ASSERT (std::find(m_entityData.begin(), m_entityData.end(), entityData) == m_entityData.end());
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

void GroupEntityData::setEntityData(const RefArray< EntityData >& entityData)
{
	m_entityData = entityData;
}

RefArray< EntityData >& GroupEntityData::getEntityData()
{
	return m_entityData;
}

const RefArray< EntityData >& GroupEntityData::getEntityData() const
{
	return m_entityData;
}

void GroupEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = transform * getTransform().inverse();
	for (RefArray< EntityData >::iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
	{
		if ((*i) != 0)
		{
			Transform currentTransform = (*i)->getTransform();
			(*i)->setTransform(deltaTransform * currentTransform);
		}
	}
	EntityData::setTransform(transform);
}

void GroupEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);
	s >> MemberRefArray< EntityData >(L"entityData", m_entityData);
}
	
	}
}
