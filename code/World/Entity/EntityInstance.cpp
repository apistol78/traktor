#include <algorithm>
#include "World/Entity/EntityInstance.h"
#include "World/Entity/EntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAggregate.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityInstance", 1, EntityInstance, ISerializable)

EntityInstance::EntityInstance()
{
}

EntityInstance::EntityInstance(const std::wstring& name, EntityData* entityData)
:	m_name(name)
,	m_entityData(entityData)
{
}

void EntityInstance::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& EntityInstance::getName() const
{
	return m_name;
}

void EntityInstance::setEntityData(EntityData* entityData)
{
	m_entityData = entityData;
}

Ref< EntityData > EntityInstance::getEntityData() const
{
	return m_entityData;
}

void EntityInstance::setInstanceData(ISerializable* instanceData)
{
	m_instanceData = instanceData;
}

Ref< ISerializable > EntityInstance::getInstanceData() const
{
	return m_instanceData;
}

void EntityInstance::addReference(EntityInstance* reference)
{
	m_references.push_back(reference);
}

void EntityInstance::removeReference(EntityInstance* reference)
{
	std::vector< EntityInstance* >::iterator i = std::find(m_references.begin(), m_references.end(), reference);
	if (i != m_references.end())
		m_references.erase(i);
}

void EntityInstance::removeAllReferences()
{
	m_references.resize(0);
}

const std::vector< EntityInstance* >& EntityInstance::getReferences() const
{
	return m_references;
}

bool EntityInstance::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberRef< EntityData >(L"entityData", m_entityData);

	if (s.getVersion() >= 1)
		s >> MemberRef< ISerializable >(L"instanceData", m_instanceData);

	s >> MemberStlVector< EntityInstance*, MemberAggregate< EntityInstance* > >(L"references", m_references);
	return true;
}

	}
}
