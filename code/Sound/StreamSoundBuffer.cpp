/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Sound/IStreamDecoder.h"
#include "Sound/StreamSoundBuffer.h"

namespace traktor::sound
{
	namespace
	{

struct StreamSoundBufferCursor : public RefCountImpl< ISoundBufferCursor >
{
	uint64_t m_position;

	StreamSoundBufferCursor()
	:	m_position(0)
	{
	}

	virtual void setParameter(handle_t id, float parameter)
	{
	}

	virtual void disableRepeat()
	{
	}

	virtual void reset()
	{
		m_position = 0;
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.StreamSoundBuffer", StreamSoundBuffer, ISoundBuffer)

StreamSoundBuffer::~StreamSoundBuffer()
{
	destroy();
}

bool StreamSoundBuffer::create(IStreamDecoder* streamDecoder)
{
	if ((m_streamDecoder = streamDecoder) != nullptr)
		return true;
	else
		return false;
}

void StreamSoundBuffer::destroy()
{
	safeDestroy(m_streamDecoder);
}

Ref< ISoundBufferCursor > StreamSoundBuffer::createCursor() const
{
	return new StreamSoundBufferCursor();
}

bool StreamSoundBuffer::getBlock(ISoundBufferCursor* cursor, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	StreamSoundBufferCursor* ssbc = static_cast< StreamSoundBufferCursor* >(cursor);
	const uint64_t position = ssbc->m_position;

	if (m_position > position)
	{
		T_DEBUG(L"Rewind stream sound decoder");
		m_streamDecoder->rewind();
		m_position = 0;
	}

	const uint32_t samplesCount = outBlock.samplesCount;
	while (m_position <= position)
	{
		outBlock.samplesCount = samplesCount;
		if (!m_streamDecoder->getBlock(outBlock))
			return false;

		m_position += outBlock.samplesCount;
	}

	ssbc->m_position += outBlock.samplesCount;
	return true;
}

}
