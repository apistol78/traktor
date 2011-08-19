#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberType.h"
#include "Database/Instance.h"
#include "Sound/Sound.h"
#include "Sound/StaticSoundBuffer.h"
#include "Sound/StaticSoundResource.h"
#include "Sound/IStreamDecoder.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.StaticSoundResource", 0, StaticSoundResource, ISoundResource)

StaticSoundResource::StaticSoundResource()
:	m_sampleRate(0)
,	m_samplesCount(0)
,	m_channelsCount(0)
,	m_decoderType(0)
{
}

Ref< Sound > StaticSoundResource::createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const
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

	Ref< StaticSoundBuffer > soundBuffer = new StaticSoundBuffer();
	if (!soundBuffer->create(m_sampleRate, m_samplesCount, m_channelsCount))
	{
		log::error << L"Failed to create sound; unable to create static sound buffer" << Endl;
		return 0;
	}

	SoundBlock soundBlock;
	std::memset(&soundBlock, 0, sizeof(soundBlock));
	soundBlock.samplesCount = 4096;

	uint32_t offset = 0;

	// Decode samples into sound buffer.
	while (streamDecoder->getBlock(soundBlock))
	{
		uint32_t samplesCount = std::min(soundBlock.samplesCount, m_samplesCount - offset);

		for (uint32_t i = 0; i < m_channelsCount; ++i)
		{
			int16_t* samples = soundBuffer->getSamplesData(i);
			T_ASSERT (samples);

			samples += offset;

			if (soundBlock.samples[i])
			{
				for (uint32_t j = 0; j < samplesCount; ++j)
					samples[j] = int16_t(soundBlock.samples[i][j] * 32767.0f);
			}
			else
			{
				for (uint32_t j = 0; j < samplesCount; ++j)
					samples[j] = 0;
			}
		}

		offset += samplesCount;
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

	return new Sound(soundBuffer);
}

bool StaticSoundResource::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"sampleRate", m_sampleRate);
	s >> Member< uint32_t >(L"samplesCount", m_samplesCount);
	s >> Member< uint32_t >(L"channelsCount", m_channelsCount);
	s >> MemberType(L"decoderType", m_decoderType);
	return true;
}

	}
}
