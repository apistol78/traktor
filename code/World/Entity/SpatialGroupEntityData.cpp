#include <algorithm>
#include "World/Entity/SpatialGroupEntityData.h"
#include "World/Entity/EntityInstance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.SpatialGroupEntityData", SpatialGroupEntityData, SpatialEntityData)

void SpatialGroupEntityData::addInstance(EntityInstance* instance)
{
	m_instances.push_back(instance);
}

void SpatialGroupEntityData::removeInstance(EntityInstance* instance)
{
	RefArray< EntityInstance >::iterator i = std::find(m_instances.begin(), m_instances.end(), instance);
	if (i != m_instances.end())
		m_instances.erase(i);
}

void SpatialGroupEntityData::removeAllInstances()
{
	m_instances.resize(0);
}

RefArray< EntityInstance >& SpatialGroupEntityData::getInstances()
{
	return m_instances;
}

const RefArray< EntityInstance >& SpatialGroupEntityData::getInstances() const
{
	return m_instances;
}

void SpatialGroupEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = getTransform().inverse() * transform;
	for (RefArray< EntityInstance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		SpatialEntityData* entityData = dynamic_type_cast< SpatialEntityData* >((*i)->getEntityData());
		if (entityData)
		{
			Transform currentTransform = entityData->getTransform();
			entityData->setTransform(currentTransform * deltaTransform);
		}
	}
	SpatialEntityData::setTransform(transform);
}

bool SpatialGroupEntityData::serialize(ISerializer& s)
{
	if (!SpatialEntityData::serialize(s))
		return false;

	s >> MemberRefArray< EntityInstance >(L"instances", m_instances);
	return true;
}
	
	}
}
