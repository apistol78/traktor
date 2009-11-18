#include "Sound/SoundFactory.h"
#include "Sound/StaticSoundResource.h"
#include "Sound/StreamSoundResource.h"
#include "Sound/SoundSystem.h"
#include "Sound/StaticSoundBuffer.h"
#include "Sound/StreamSoundBuffer.h"
#include "Sound/IStreamDecoder.h"
#include "Sound/Sound.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundFactory", SoundFactory, resource::IResourceFactory)

SoundFactory::SoundFactory(db::Database* db, SoundSystem* soundSystem)
:	m_db(db)
,	m_soundSystem(soundSystem)
{
}

const TypeInfoSet SoundFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Sound >());
	return typeSet;
}

bool SoundFactory::isCacheable() const
{
	return true;
}

Ref< Object > SoundFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< SoundResource > resource = instance->getObject< SoundResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	Ref< Sound > sound;

	if (is_a< StaticSoundResource >(resource))
	{
		Reader reader(stream);

		uint32_t version;
		reader >> version;
		if (version != 2)
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

		for (uint32_t i = 0; i < channelsCount; ++i)
		{
			int16_t* samples = soundBuffer->getSamplesData(i);
			T_ASSERT (samples);

			if (reader.read(samples, samplesCount, sizeof(int16_t)) != samplesCount * sizeof(int16_t))
				log::warning << L"Unable to read specified number of samples into channel " << i << Endl;
		}

		stream->close();

		sound = new Sound(soundBuffer);
	}
	else if (StreamSoundResource* streamResource = dynamic_type_cast< StreamSoundResource* >(resource))
	{
		Ref< IStreamDecoder > streamDecoder = checked_type_cast< IStreamDecoder* >(streamResource->getDecoderType()->createInstance());
		if (!streamDecoder->create(stream))
		{
			log::error << L"Unable to create sound, unable to create stream decoder" << Endl;
			return 0;
		}

		Ref< StreamSoundBuffer > soundBuffer = new StreamSoundBuffer();
		if (!soundBuffer->create(streamDecoder))
		{
			log::error << L"Unable to create sound, unable to create stream sound buffer" << Endl;
			return 0;
		}

		sound = new Sound(soundBuffer);
	}

	return sound;
}

	}
}
