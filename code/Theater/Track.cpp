#include "Theater/Track.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.Track", Track, Object)

Track::Track(world::SpatialEntity* entity, const TransformPath& path)
:	m_entity(entity)
,	m_path(path)
{
}

void Track::setEntity(world::SpatialEntity* entity)
{
	m_entity = entity;
}

world::SpatialEntity* Track::getEntity() const
{
	return m_entity;
}

const TransformPath& Track::getPath() const
{
	return m_path;
}

TransformPath& Track::getPath()
{
	return m_path;
}

	}
}
