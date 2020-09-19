#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/EntityData.h"
#include "World/Entity/FacadeComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.FacadeComponentData", 0, FacadeComponentData, IEntityComponentData)

void FacadeComponentData::addEntityData(EntityData* entityData)
{
	T_ASSERT(std::find(m_entityData.begin(), m_entityData.end(), entityData) == m_entityData.end());
	m_entityData.push_back(entityData);
}

void FacadeComponentData::removeEntityData(EntityData* entityData)
{
	auto it = std::find(m_entityData.begin(), m_entityData.end(), entityData);
	if (it != m_entityData.end())
		m_entityData.erase(it);
}

void FacadeComponentData::removeAllEntityData()
{
	m_entityData.resize(0);
}

void FacadeComponentData::setEntityData(const RefArray< EntityData >& entityData)
{
	m_entityData = entityData;
}

RefArray< EntityData >& FacadeComponentData::getEntityData()
{
	return m_entityData;
}

const RefArray< EntityData >& FacadeComponentData::getEntityData() const
{
	return m_entityData;
}

const std::wstring& FacadeComponentData::getShow() const
{
	return m_show;
}

void FacadeComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
	Transform invTransform = owner->getTransform().inverse();
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
}

void FacadeComponentData::serialize(ISerializer& s)
{
    s >> MemberRefArray< EntityData >(L"entityData", m_entityData, AttributePrivate());
	s >> Member< std::wstring >(L"show", m_show);
}

	}
}
