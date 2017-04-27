/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TrackData", 5, TrackData, ISerializable)

TrackData::TrackData()
:	m_loopStart(0.0f)
,	m_loopEnd(0.0f)
,	m_timeOffset(0.0f)
,	m_wobbleMagnitude(0.0f)
,	m_wobbleRate(0.0f)
{
}

void TrackData::setEntityData(world::EntityData* entityData)
{
	m_entityData = entityData;
}

Ref< world::EntityData > TrackData::getEntityData() const
{
	return m_entityData;
}

void TrackData::setLookAtEntityData(world::EntityData* entityData)
{
	m_lookAtEntityData = entityData;
}

Ref< world::EntityData > TrackData::getLookAtEntityData() const
{
	return m_lookAtEntityData;
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
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);

	if (s.getVersion() >= 2)
		s >> MemberRef< world::EntityData >(L"lookAtEntityData", m_lookAtEntityData);

	s >> MemberComposite< TransformPath >(L"path", m_path);
	
	if (s.getVersion() >= 1)
	{
		s >> Member< float >(L"loopStart", m_loopStart);
		s >> Member< float >(L"loopEnd", m_loopEnd);

		if (s.getVersion() < 3)
		{
			float loopEase = 0.0f;
			s >> Member< float >(L"loopEase", loopEase);
		}
	}

	if (s.getVersion() >= 4)
		s >> Member< float >(L"timeOffset", m_timeOffset);

	if (s.getVersion() >= 5)
	{
		s >> Member< float >(L"wobbleMagnitude", m_wobbleMagnitude, AttributeRange(0.0f));
		s >> Member< float >(L"wobbleRate", m_wobbleRate, AttributeRange(0.0f));
	}
}

	}
}
