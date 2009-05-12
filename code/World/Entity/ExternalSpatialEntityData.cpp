#include "World/Entity/ExternalSpatialEntityData.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.ExternalSpatialEntityData", ExternalSpatialEntityData, SpatialEntityData)

ExternalSpatialEntityData::ExternalSpatialEntityData()
{
}

ExternalSpatialEntityData::ExternalSpatialEntityData(const Guid& guid)
:	m_guid(guid)
{
}

void ExternalSpatialEntityData::setGuid(const Guid& guid)
{
	m_guid = guid;
}

const Guid& ExternalSpatialEntityData::getGuid() const
{
	return m_guid;
}

bool ExternalSpatialEntityData::serialize(Serializer& s)
{
	if (!SpatialEntityData::serialize(s))
		return false;

	return s >> Member< Guid >(L"guid", m_guid, &type_of< SpatialEntityData >());
}

	}
}
