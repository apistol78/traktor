#include "Compress/Zip/InflateStream.h"
#include "Core/Io/BitReader.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Endian.h"
#include "Database/Instance.h"
#include "Sound/Delta.h"
#include "Sound/Sound.h"
#include "Sound/StaticSoundBuffer.h"
#include "Sound/StaticSoundResource.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.StaticSoundResource", 0, StaticSoundResource, ISoundResource)

Ref< Sound > StaticSoundResource::createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const
{
	Ref< IStream > stream = resourceInstance->readData(L"Data");
	if (!stream)
		return 0;

	Reader reader(stream);

	uint32_t version;
	reader >> version;
	if (version != 5)
	{
		log::error << L"Failed to create sound; incorrect version" << Endl;
		return 0;
	}

	uint32_t sampleRate, samplesCount, channelsCount;
	uint8_t flags;

	reader >> sampleRate;
	reader >> samplesCount;
	reader >> channelsCount;
	reader >> flags;

	Ref< StaticSoundBuffer > soundBuffer = new StaticSoundBuffer();
	if (!soundBuffer->create(sampleRate, samplesCount, channelsCount))
	{
		log::error << L"Failed to create sound; unable to create static sound buffer" << Endl;
		return 0;
	}

	Ref< IStream > streamData;
	if (flags & SrfZLib)
		streamData = new compress::InflateStream(stream);
	else
		streamData = stream;

	for (uint32_t i = 0; i < channelsCount; ++i)
	{
		int16_t* samples = soundBuffer->getSamplesData(i);
		T_ASSERT (samples);

		if (flags & SrfDelta)
		{
			BitReader br(streamData);
			deltaDecode(br, samplesCount, samples);
		}
		else
		{
			if (streamData->read(samples, samplesCount * sizeof(int16_t)) != samplesCount * sizeof(int16_t))
			{
				log::error << L"Failed to create sound; unable to read samples" << Endl;
				return 0;
			}

#if !defined(T_LITTLE_ENDIAN)
			for (uint32_t i = 0; i < samplesCount; ++i)
				swap8in32(samples[i]);
#endif
		}
	}

	streamData->close();
	stream->close();

	return new Sound(soundBuffer);
}

bool StaticSoundResource::serialize(ISerializer& s)
{
	return true;
}

	}
}
