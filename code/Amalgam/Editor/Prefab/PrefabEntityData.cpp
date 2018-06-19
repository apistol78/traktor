#include "Amalgam/Editor/Prefab/PrefabEntityData.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.PrefabEntityData", 0, PrefabEntityData, world::EntityData)

PrefabEntityData::PrefabEntityData()
:	m_partitionMesh(false)
{
	m_outputGuid[0] = Guid::create();
	m_outputGuid[1] = Guid::create();
}

void PrefabEntityData::addEntityData(world::EntityData* entityData)
{
	T_ASSERT (std::find(m_entityData.begin(), m_entityData.end(), entityData) == m_entityData.end());
	m_entityData.push_back(entityData);
}

void PrefabEntityData::removeEntityData(world::EntityData* entityData)
{
	RefArray< world::EntityData >::iterator i = std::find(m_entityData.begin(), m_entityData.end(), entityData);
	if (i != m_entityData.end())
		m_entityData.erase(i);
}

void PrefabEntityData::removeAllEntityData()
{
	m_entityData.resize(0);
}

void PrefabEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = transform * getTransform().inverse();
	for (RefArray< world::EntityData >::iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
	{
		Transform currentTransform = (*i)->getTransform();
		(*i)->setTransform(deltaTransform * currentTransform);
	}
	world::EntityData::setTransform(transform);
}

void PrefabEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);
	
	s >> MemberStaticArray< Guid, 2 >(L"outputGuid", m_outputGuid, AttributePrivate());
	s >> MemberRefArray< world::EntityData >(L"entityData", m_entityData);
	s >> Member< bool >(L"partitionMesh", m_partitionMesh);
}

	}
}
