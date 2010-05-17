#include "Compress/Zip/InflateStream.h"
#include "Core/Io/BitReader.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
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
	if (version != 4)
	{
		log::error << L"Unable to create sound, incorrect version" << Endl;
		return 0;
	}

	uint32_t sampleRate, samplesCount, channelsCount;
	reader >> sampleRate;
	reader >> samplesCount;
	reader >> channelsCount;

	Ref< StaticSoundBuffer > soundBuffer = new StaticSoundBuffer();
	if (!soundBuffer->create(sampleRate, samplesCount, channelsCount))
	{
		log::error << L"Unable to create sound, unable to create static sound buffer" << Endl;
		return 0;
	}

	Ref< compress::InflateStream > streamData = new compress::InflateStream(stream);
	for (uint32_t i = 0; i < channelsCount; ++i)
	{
		int16_t* samples = soundBuffer->getSamplesData(i);
		T_ASSERT (samples);

		BitReader br(streamData);
		deltaDecode(br, samplesCount, samples);
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
