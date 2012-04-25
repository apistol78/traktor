#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ExternalEntityData", 1, ExternalEntityData, EntityData)

ExternalEntityData::ExternalEntityData()
{
}

ExternalEntityData::ExternalEntityData(const resource::Id< EntityData >& entityData)
:	m_entityData(entityData)
{
}

void ExternalEntityData::setEntityData(const resource::Id< EntityData >& entityData)
{
	m_entityData = entityData;
}

const resource::Id< EntityData >& ExternalEntityData::getEntityData() const
{
	return m_entityData;
}

bool ExternalEntityData::serialize(ISerializer& s)
{
	if (!EntityData::serialize(s))
		return false;

	if (s.getVersion() >= 1)
		return s >> resource::Member< EntityData >(L"entityData", m_entityData);
	else
		return s >> resource::Member< EntityData >(L"guid", m_entityData);
}

	}
}
