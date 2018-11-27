#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Sound.h"
#include "Sound/Tracker/PatternData.h"
#include "Sound/Tracker/SongBuffer.h"
#include "Sound/Tracker/SongResource.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SongResource", 0, SongResource, ISoundResource)

SongResource::SongResource()
:	m_gain(0.0f)
,	m_presence(0.0f)
,	m_presenceRate(1.0f)
,	m_range(0.0f)
,	m_bpm(120)
{
}

SongResource::SongResource(
	const RefArray< const PatternData >& patterns,
	const std::wstring& category,
	float gain,
	float presence,
	float presenceRate,
	float range,
	int32_t bpm
)
:	m_patterns(patterns)
,	m_category(category)
,	m_gain(gain)
,	m_presence(presence)
,	m_presenceRate(presenceRate)
,	m_range(range)
,	m_bpm(bpm)
{
}

Ref< Sound > SongResource::createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const
{
	RefArray< Pattern > patterns;
	for (auto patternData : m_patterns)
	{
		Ref< Pattern > pattern = patternData->createInstance(resourceManager);
		if (!pattern)
			return nullptr;

		patterns.push_back(pattern);
	}

	return new Sound(
		new SongBuffer(patterns, m_bpm),
		getParameterHandle(m_category),
		m_gain,
		m_presence,
		m_presenceRate,
		m_range
	);
}

void SongResource::serialize(ISerializer& s)
{
	s >> MemberRefArray< const PatternData >(L"patterns", m_patterns);
	s >> Member< std::wstring >(L"category", m_category);
	s >> Member< float >(L"gain", m_gain);
	s >> Member< float >(L"presence", m_presence);
	s >> Member< float >(L"presenceRate", m_presenceRate);
	s >> Member< float >(L"range", m_range);
	s >> Member< int32_t >(L"bpm", m_bpm);
}

	}
}
