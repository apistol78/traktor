#include <algorithm>
#include "World/Entity/EntityInstance.h"
#include "World/Entity/EntityData.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.EntityInstance", EntityInstance, Serializable)

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

EntityData* EntityInstance::getEntityData() const
{
	return m_entityData;
}

void EntityInstance::setInstanceData(Serializable* instanceData)
{
	m_instanceData = instanceData;
}

Serializable* EntityInstance::getInstanceData() const
{
	return m_instanceData;
}

void EntityInstance::addReference(EntityInstance* reference)
{
	m_references.push_back(reference);
}

void EntityInstance::removeReference(EntityInstance* reference)
{
	RefArray< EntityInstance >::iterator i = std::find(m_references.begin(), m_references.end(), reference);
	if (i != m_references.end())
		m_references.erase(i);
}

void EntityInstance::removeAllReferences()
{
	m_references.resize(0);
}

const RefArray< EntityInstance >& EntityInstance::getReferences() const
{
	return m_references;
}

int EntityInstance::getVersion() const
{
	return 1;
}

bool EntityInstance::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberRef< EntityData >(L"entityData", m_entityData);

	if (s.getVersion() >= 1)
		s >> MemberRef< Serializable >(L"instanceData", m_instanceData);

	s >> MemberRefArray< EntityInstance >(L"references", m_references);
	return true;
}

	}
}
