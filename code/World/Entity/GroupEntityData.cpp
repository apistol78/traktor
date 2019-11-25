#include <algorithm>
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/MemberBitMask.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/WorldTypes.h"
#include "World/Entity/GroupEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.GroupEntityData", 1, GroupEntityData, EntityData)

GroupEntityData::GroupEntityData()
:	m_mask(world::EmAll)
{
}

void GroupEntityData::setMask(uint32_t mask)
{
	m_mask = mask;
}

uint32_t GroupEntityData::getMask() const
{
	return m_mask;
}

void GroupEntityData::addEntityData(EntityData* entityData)
{
	T_ASSERT(std::find(m_entityData.begin(), m_entityData.end(), entityData) == m_entityData.end());
	m_entityData.push_back(entityData);
}

void GroupEntityData::removeEntityData(EntityData* entityData)
{
	auto i = std::find(m_entityData.begin(), m_entityData.end(), entityData);
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
	Transform invTransform = getTransform().inverse();
	for (auto entityData : m_entityData)
	{
		if (entityData != nullptr)
		{
			Transform currentTransform = entityData->getTransform();
			Transform Tlocal = invTransform * currentTransform;
			Transform Tworld = transform * Tlocal;
			entityData->setTransform(Tworld);
		}
	}
	EntityData::setTransform(transform);
}

void GroupEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);
	
	if (s.getVersion< GroupEntityData >() >= 1)
	{
		const MemberBitMask::Bit kMaskBits[] =
		{
			{ L"static", EmStatic },
			{ L"dynamic", EmDynamic },
			{ 0 }
		};		
		s >> MemberBitMask(L"mask", m_mask, kMaskBits);
	}

	s >> MemberRefArray< EntityData >(L"entityData", m_entityData, AttributePrivate());
}

	}
}
