#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Theater/TrackData.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TrackData", 6, TrackData, ISerializable)

TrackData::TrackData()
:	m_loopStart(0.0f)
,	m_loopEnd(0.0f)
,	m_timeOffset(0.0f)
,	m_wobbleMagnitude(0.0f)
,	m_wobbleRate(0.0f)
{
}

void TrackData::setEntityId(const Guid& entityId)
{
	m_entityId = entityId;
}

const Guid& TrackData::getEntityId() const
{
	return m_entityId;
}

void TrackData::setLookAtEntityId(const Guid& entityId)
{
	m_lookAtEntityId = entityId;
}

const Guid& TrackData::getLookAtEntityId() const
{
	return m_lookAtEntityId;
}

void TrackData::setPath(const TransformPath& path)
{
	m_path = path;
}

const TransformPath& TrackData::getPath() const
{
	return m_path;
}

TransformPath& TrackData::getPath()
{
	return m_path;
}

void TrackData::setLoopStart(float loopStart)
{
	m_loopStart = loopStart;
}

float TrackData::getLoopStart() const
{
	return m_loopStart;
}

void TrackData::setLoopEnd(float loopEnd)
{
	m_loopEnd = loopEnd;
}

float TrackData::getLoopEnd() const
{
	return m_loopEnd;
}

void TrackData::setTimeOffset(float timeOffset)
{
	m_timeOffset = timeOffset;
}

float TrackData::getTimeOffset() const
{
	return m_timeOffset;
}

void TrackData::setWobbleMagnitude(float wobbleMagnitude)
{
	m_wobbleMagnitude = wobbleMagnitude;
}

float TrackData::getWobbleMagnitude() const
{
	return m_wobbleMagnitude;
}

void TrackData::setWobbleRate(float wobbleRate)
{
	m_wobbleRate = wobbleRate;
}

float TrackData::getWobbleRate() const
{
	return m_wobbleRate;
}

void TrackData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< TrackData >() >= 6);

	s >> Member< Guid >(L"entityId", m_entityId, AttributePrivate());
	s >> Member< Guid >(L"lookAtEntityId", m_lookAtEntityId, AttributePrivate());
	s >> MemberComposite< TransformPath >(L"path", m_path);
	s >> Member< float >(L"loopStart", m_loopStart);
	s >> Member< float >(L"loopEnd", m_loopEnd);
	s >> Member< float >(L"timeOffset", m_timeOffset);
	s >> Member< float >(L"wobbleMagnitude", m_wobbleMagnitude, AttributeRange(0.0f));
	s >> Member< float >(L"wobbleRate", m_wobbleRate, AttributeRange(0.0f));
}

	}
}
