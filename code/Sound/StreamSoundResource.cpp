#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberType.h"
#include "Database/Instance.h"
#include "Sound/IStreamDecoder.h"
#include "Sound/Sound.h"
#include "Sound/StreamSoundBuffer.h"
#include "Sound/StreamSoundResource.h"

#if defined(_PS3)
#	define T_PRELOAD_STREAM_DATA	1
#endif

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.StreamSoundResource", 0, StreamSoundResource, ISoundResource)

StreamSoundResource::StreamSoundResource(const TypeInfo* decoderType)
:	m_decoderType(decoderType)
{
}

Ref< Sound > StreamSoundResource::createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const
{
	Ref< IStream > stream = resourceInstance->readData(L"Data");
	if (!stream)
		return 0;

	if (!m_decoderType)
	{
		log::error << L"Unable to create sound, no decoder type" << Endl;
		return 0;
	}

#if T_PRELOAD_STREAM_DATA

	Ref< DynamicMemoryStream > memoryStream = new DynamicMemoryStream();
	if (!StreamCopy(memoryStream, stream).execute())
	{
		log::error << L"Unable to create sound, pre-loading failed" << Endl;
		return 0;
	}

	stream->close();

	stream = memoryStream;
	stream->seek(IStream::SeekSet, 0);

#endif

	Ref< IStreamDecoder > streamDecoder = checked_type_cast< IStreamDecoder* >(m_decoderType->createInstance());
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

	return new Sound(soundBuffer);
}

bool StreamSoundResource::serialize(ISerializer& s)
{
	return s >> MemberType(L"decoderType", m_decoderType);
}

	}
}
