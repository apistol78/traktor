#include <algorithm>
#include "World/Entity/SpatialGroupEntityData.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.SpatialGroupEntityData", SpatialGroupEntityData, SpatialEntityData)

void SpatialGroupEntityData::addEntityData(SpatialEntityData* entityData)
{
	m_entityData.push_back(entityData);
}

void SpatialGroupEntityData::removeEntityData(SpatialEntityData* entityData)
{
	RefArray< SpatialEntityData >::iterator i = std::find(m_entityData.begin(), m_entityData.end(), entityData);
	if (i != m_entityData.end())
		m_entityData.erase(i);
}

RefArray< SpatialEntityData >& SpatialGroupEntityData::getEntityData()
{
	return m_entityData;
}

const RefArray< SpatialEntityData >& SpatialGroupEntityData::getEntityData() const
{
	return m_entityData;
}

void SpatialGroupEntityData::setTransform(const Matrix44& transform)
{
	Matrix44 deltaTransform = getTransform().inverseOrtho() * transform;
	for (RefArray< SpatialEntityData >::iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
	{
		Matrix44 currentTransform = (*i)->getTransform();
		(*i)->setTransform(currentTransform * deltaTransform);
	}
	SpatialEntityData::setTransform(transform);
}

bool SpatialGroupEntityData::serialize(Serializer& s)
{
	if (!SpatialEntityData::serialize(s))
		return false;

	return s >> MemberRefArray< SpatialEntityData >(L"entityData", m_entityData);
}
	
	}
}
