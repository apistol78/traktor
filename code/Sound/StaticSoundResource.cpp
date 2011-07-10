#include "Compress/Lzf/InflateStreamLzf.h"
#include "Compress/Lzo/InflateStreamLzo.h"
#include "Compress/Zip/InflateStreamZip.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberBitMask.h"
#include "Database/Instance.h"
#include "Sound/Sound.h"
#include "Sound/StaticSoundBuffer.h"
#include "Sound/StaticSoundResource.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.StaticSoundResource", 0, StaticSoundResource, ISoundResource)

StaticSoundResource::StaticSoundResource()
:	m_sampleRate(0)
,	m_samplesCount(0)
,	m_channelsCount(0)
,	m_flags(0)
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

	Ref< StaticSoundBuffer > soundBuffer = new StaticSoundBuffer();
	if (!soundBuffer->create(m_sampleRate, m_samplesCount, m_channelsCount))
	{
		log::error << L"Failed to create sound; unable to create static sound buffer" << Endl;
		return 0;
	}

	Ref< IStream > streamData;
	if (m_flags & SrfZLib)
		streamData = new compress::InflateStreamZip(stream);
	else if (m_flags & SrfLzo)
		streamData = new compress::InflateStreamLzo(stream);
	else if (m_flags & SrfLzf)
		streamData = new compress::InflateStreamLzf(stream);
	else
		streamData = stream;

	for (uint32_t i = 0; i < m_channelsCount; ++i)
	{
		int16_t* samples = soundBuffer->getSamplesData(i);
		T_ASSERT (samples);

		if (streamData->read(samples, m_samplesCount * sizeof(int16_t)) != m_samplesCount * sizeof(int16_t))
		{
			log::error << L"Failed to create sound; unable to read samples" << Endl;
			return 0;
		}
	}

	streamData->close();
	stream->close();

	return new Sound(soundBuffer);
}

bool StaticSoundResource::serialize(ISerializer& s)
{
	const MemberBitMask::Bit c_Flags_bits[] =
	{
		{ L"SrfZLib", SrfZLib },
		{ 0 }
	};

	s >> Member< uint32_t >(L"sampleRate", m_sampleRate);
	s >> Member< uint32_t >(L"samplesCount", m_samplesCount);
	s >> Member< uint32_t >(L"channelsCount", m_channelsCount);
	s >> MemberBitMask(L"flags", m_flags, c_Flags_bits);
	
	return true;
}

	}
}
