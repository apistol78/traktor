#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ExternalEntityData", 0, ExternalEntityData, EntityData)

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

bool ExternalEntityData::serialize(ISerializer& s)
{
	if (!EntityData::serialize(s))
		return false;

	return s >> Member< Guid >(
		L"guid",
		m_guid,
		AttributeType(type_of< EntityData >())
	);
}

	}
}
