/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/MemoryStream.h"
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

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.StreamSoundResource", 8, StreamSoundResource, ISoundResource)

Ref< Sound > StreamSoundResource::createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const
{
	Ref< IStream > stream = resourceInstance->readData(L"Data");
	if (!stream)
		return nullptr;

	if (!m_decoderType)
	{
		log::error << L"Unable to create sound, no decoder type." << Endl;
		return nullptr;
	}

	if (m_preload)
	{
		const int64_t size = stream->available();

		uint8_t* buffer = new uint8_t [size];
		for (int64_t i = 0; i < size; )
		{
			const int64_t res = stream->read(&buffer[i], size - i);
			if (res <= 0)
				return nullptr;
			i += res;
		}

		stream = new MemoryStream(buffer, size, true, false, true);
	}

	Ref< IStreamDecoder > streamDecoder = checked_type_cast< IStreamDecoder* >(m_decoderType->createInstance());
	if (!streamDecoder->create(stream))
	{
		log::error << L"Unable to create sound, unable to create stream decoder." << Endl;
		return nullptr;
	}

	Ref< StreamSoundBuffer > soundBuffer = new StreamSoundBuffer();
	if (!soundBuffer->create(streamDecoder))
	{
		log::error << L"Unable to create sound, unable to create stream sound buffer." << Endl;
		return nullptr;
	}

	return new Sound(
		soundBuffer,
		getParameterHandle(m_category),
		m_gain,
		m_range
	);
}

void StreamSoundResource::serialize(ISerializer& s)
{
	T_FATAL_ASSERT (s.getVersion() >= 8);
	s >> MemberType(L"decoderType", m_decoderType);
	s >> Member< std::wstring >(L"category", m_category);
	s >> Member< float >(L"gain", m_gain);
	s >> Member< float >(L"range", m_range);
	s >> Member< bool >(L"preload", m_preload);
}

}
