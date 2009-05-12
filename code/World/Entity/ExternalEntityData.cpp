#include "World/Entity/ExternalEntityData.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.ExternalEntityData", ExternalEntityData, EntityData)

ExternalEntityData::ExternalEntityData()
{
}

ExternalEntityData::ExternalEntityData(const Guid& guid)
:	m_guid(guid)
{
}

void ExternalEntityData::setGuid(const Guid& guid)
{
	m_guid = guid;
}

const Guid& ExternalEntityData::getGuid() const
{
	return m_guid;
}

bool ExternalEntityData::serialize(Serializer& s)
{
	if (!EntityData::serialize(s))
		return false;

	return s >> Member< Guid >(L"guid", m_guid, &type_of< EntityData >());
}

	}
}
