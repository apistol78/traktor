#include <algorithm>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/Entity/SwitchEntityData.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.SwitchEntityData", 0, SwitchEntityData, EntityData)

void SwitchEntityData::addEntityData(EntityData* entityData)
{
	T_ASSERT (std::find(m_entityData.begin(), m_entityData.end(), entityData) == m_entityData.end());
	m_entityData.push_back(entityData);
}

void SwitchEntityData::removeEntityData(EntityData* entityData)
{
	RefArray< EntityData >::iterator i = std::find(m_entityData.begin(), m_entityData.end(), entityData);
	if (i != m_entityData.end())
		m_entityData.erase(i);
}

void SwitchEntityData::removeAllEntityData()
{
	m_entityData.resize(0);
}

void SwitchEntityData::setEntityData(const RefArray< EntityData >& entityData)
{
	m_entityData = entityData;
}

RefArray< EntityData >& SwitchEntityData::getEntityData()
{
	return m_entityData;
}

const RefArray< EntityData >& SwitchEntityData::getEntityData() const
{
	return m_entityData;
}

void SwitchEntityData::setTransform(const Transform& transform)
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

void SwitchEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);
	s >> MemberRefArray< EntityData >(L"entityData", m_entityData);
}
	
	}
}
