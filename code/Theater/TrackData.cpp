#include "Theater/TrackData.h"
#include "World/Entity/EntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TrackData", 0, TrackData, ISerializable)

void TrackData::setEntityData(world::EntityData* entityData)
{
	m_entityData = entityData;
}

Ref< world::EntityData > TrackData::getEntityData() const
{
	return m_entityData;
}

const TransformPath& TrackData::getPath() const
{
	return m_path;
}

TransformPath& TrackData::getPath()
{
	return m_path;
}

bool TrackData::serialize(ISerializer& s)
{
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
	s >> MemberComposite< TransformPath >(L"path", m_path);
	return true;
}

	}
}
