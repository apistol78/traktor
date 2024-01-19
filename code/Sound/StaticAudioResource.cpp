/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cstring>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberType.h"
#include "Database/Instance.h"
#include "Sound/Sound.h"
#include "Sound/StaticAudioBuffer.h"
#include "Sound/StaticAudioResource.h"
#include "Sound/IStreamDecoder.h"

#undef min
#undef max

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.StaticAudioResource", 7, StaticAudioResource, IAudioResource)

StaticAudioResource::StaticAudioResource()
:	m_sampleRate(0)
,	m_samplesCount(0)
,	m_channelsCount(0)
,	m_gain(0.0f)
,	m_range(0.0f)
,	m_decoderType(0)
{
}

Ref< Sound > StaticAudioResource::createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const
{
	Ref< IStream > stream = resourceInstance->readData(L"Data");
	if (!stream)
	{
		log::error << L"Failed to create sound; no data" << Endl;
		return 0;
	}

	Ref< IStreamDecoder > streamDecoder = checked_type_cast< IStreamDecoder* >(m_decoderType->createInstance());
	if (!streamDecoder->create(stream))
	{
		log::error << L"Unable to create sound, unable to create stream decoder" << Endl;
		return 0;
	}

	Ref< StaticAudioBuffer > soundBuffer = new StaticAudioBuffer();
	if (!soundBuffer->create(m_sampleRate, m_samplesCount, m_channelsCount))
	{
		log::error << L"Failed to create sound; unable to create static sound buffer" << Endl;
		return 0;
	}

	AudioBlock block;
	std::memset(&block, 0, sizeof(block));
	block.samplesCount = 4096;

	uint32_t offset = 0;

	// Decode samples into sound buffer.
	while (streamDecoder->getBlock(block))
	{
		uint32_t samplesCount = std::min(block.samplesCount, m_samplesCount - offset);

		for (uint32_t i = 0; i < m_channelsCount; ++i)
		{
			int16_t* samples = soundBuffer->getSamplesData(i);
			T_ASSERT(samples);

			samples += offset;

			if (block.samples[i])
			{
				for (uint32_t j = 0; j < samplesCount; ++j)
				{
					float sample = clamp(block.samples[i][j], -1.0f, 1.0f);
					samples[j] = int16_t(sample * 32750.0f);
				}
			}
			else
			{
				for (uint32_t j = 0; j < samplesCount; ++j)
					samples[j] = 0;
			}
		}

		offset += samplesCount;

		std::memset(&block, 0, sizeof(block));
		block.samplesCount = 4096;
	}

	// Make sure samples are zero;ed out if not fully decoded.
	for (; offset < m_samplesCount; ++offset)
	{
		for (uint32_t i = 0; i < m_channelsCount; ++i)
		{
			int16_t* samples = soundBuffer->getSamplesData(i);
			samples[offset] = 0;
		}
	}

	stream->close();
	safeDestroy(streamDecoder);

	return new Sound(
		soundBuffer,
		getParameterHandle(m_category),
		m_gain,
		m_range
	);
}

void StaticAudioResource::serialize(ISerializer& s)
{
	T_FATAL_ASSERT (s.getVersion() >= 7);
	s >> Member< std::wstring >(L"category", m_category);
	s >> Member< uint32_t >(L"sampleRate", m_sampleRate);
	s >> Member< uint32_t >(L"samplesCount", m_samplesCount);
	s >> Member< uint32_t >(L"channelsCount", m_channelsCount);
	s >> Member< float >(L"gain", m_gain);
	s >> Member< float >(L"range", m_range);
	s >> MemberType(L"decoderType", m_decoderType);
}

}
