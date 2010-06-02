#include "Theater/Track.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.Track", Track, Object)

Track::Track(
	world::SpatialEntity* entity,
	world::SpatialEntity* lookAtEntity,
	const TransformPath& path,
	float loopStart,
	float loopEnd,
	float loopEase
)
:	m_entity(entity)
,	m_lookAtEntity(lookAtEntity)
,	m_path(path)
,	m_loopStart(loopStart)
,	m_loopEnd(loopEnd)
,	m_loopEase(loopEase)
{
}

void Track::setEntity(world::SpatialEntity* entity)
{
	m_entity = entity;
}

Ref< world::SpatialEntity > Track::getEntity() const
{
	return m_entity;
}

void Track::setLookAtEntity(world::SpatialEntity* entity)
{
	m_lookAtEntity = entity;
}

Ref< world::SpatialEntity > Track::getLookAtEntity() const
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

float Track::getLoopEase() const
{
	return m_loopEase;
}

	}
}
