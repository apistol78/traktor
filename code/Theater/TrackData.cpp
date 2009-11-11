#include "Theater/TrackData.h"
#include "World/Entity/EntityInstance.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.theater.TrackData", TrackData, Serializable)

void TrackData::setInstance(world::EntityInstance* instance)
{
	m_instance = instance;
}

Ref< world::EntityInstance > TrackData::getInstance() const
{
	return m_instance;
}

const TransformPath& TrackData::getPath() const
{
	return m_path;
}

TransformPath& TrackData::getPath()
{
	return m_path;
}

bool TrackData::serialize(Serializer& s)
{
	s >> MemberRef< world::EntityInstance >(L"instance", m_instance);
	s >> MemberComposite< TransformPath >(L"path", m_path);
	return true;
}

	}
}
