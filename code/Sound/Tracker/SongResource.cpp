/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SongResource", 0, SongResource, IAudioResource)

SongResource::SongResource()
:	m_gain(0.0f)
,	m_range(0.0f)
,	m_bpm(120)
{
}

SongResource::SongResource(
	const RefArray< const PatternData >& patterns,
	const std::wstring& category,
	float gain,
	float range,
	int32_t bpm
)
:	m_patterns(patterns)
,	m_category(category)
,	m_gain(gain)
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
		m_range
	);
}

void SongResource::serialize(ISerializer& s)
{
	s >> MemberRefArray< const PatternData >(L"patterns", m_patterns);
	s >> Member< std::wstring >(L"category", m_category);
	s >> Member< float >(L"gain", m_gain);
	s >> Member< float >(L"range", m_range);
	s >> Member< int32_t >(L"bpm", m_bpm);
}

	}
}
