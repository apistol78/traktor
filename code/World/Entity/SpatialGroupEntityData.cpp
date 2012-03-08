#include <algorithm>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/Entity/SpatialGroupEntityData.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.SpatialGroupEntityData", 0, SpatialGroupEntityData, SpatialEntityData)

void SpatialGroupEntityData::addEntityData(SpatialEntityData* entityData)
{
	T_ASSERT (std::find(m_entityData.begin(), m_entityData.end(), entityData) == m_entityData.end());
	m_entityData.push_back(entityData);
}

void SpatialGroupEntityData::removeEntityData(SpatialEntityData* entityData)
{
	RefArray< SpatialEntityData >::iterator i = std::find(m_entityData.begin(), m_entityData.end(), entityData);
	if (i != m_entityData.end())
		m_entityData.erase(i);
}

void SpatialGroupEntityData::removeAllEntityData()
{
	m_entityData.resize(0);
}

RefArray< SpatialEntityData >& SpatialGroupEntityData::getEntityData()
{
	return m_entityData;
}

const RefArray< SpatialEntityData >& SpatialGroupEntityData::getEntityData() const
{
	return m_entityData;
}

void SpatialGroupEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = transform * getTransform().inverse();
	for (RefArray< SpatialEntityData >::iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
	{
		Transform currentTransform = (*i)->getTransform();
		(*i)->setTransform(deltaTransform * currentTransform);
	}
	SpatialEntityData::setTransform(transform);
}

bool SpatialGroupEntityData::serialize(ISerializer& s)
{
	if (!SpatialEntityData::serialize(s))
		return false;

	if (!(s >> MemberRefArray< SpatialEntityData >(L"entityData", m_entityData)))
		return false;

	return true;
}
	
	}
}
