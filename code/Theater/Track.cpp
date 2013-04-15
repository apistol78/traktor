#include "Theater/Track.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.Track", Track, Object)

Track::Track(
	world::Entity* entity,
	world::Entity* lookAtEntity,
	const TransformPath& path,
	float loopStart,
	float loopEnd
)
:	m_entity(entity)
,	m_lookAtEntity(lookAtEntity)
,	m_path(path)
,	m_loopStart(loopStart)
,	m_loopEnd(loopEnd)
{
}

void Track::setEntity(world::Entity* entity)
{
	m_entity = entity;
}

Ref< world::Entity > Track::getEntity() const
{
	return m_entity;
}

void Track::setLookAtEntity(world::Entity* entity)
{
	m_lookAtEntity = entity;
}

Ref< world::Entity > Track::getLookAtEntity() const
{
	return m_lookAtEntity;
}

const TransformPath& Track::getPath() const
{
	return m_path;
}

TransformPath& Track::getPath()
{
	return m_path;
}

float Track::getLoopStart() const
{
	return m_loopStart;
}

float Track::getLoopEnd() const
{
	return m_loopEnd;
}

	}
}
