/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Sound/IStreamDecoder.h"
#include "Sound/StreamAudioBuffer.h"

namespace traktor::sound
{
	namespace
	{

struct StreamAudioBufferCursor : public RefCountImpl< IAudioBufferCursor >
{
	uint64_t m_position = 0;

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.StreamAudioBuffer", StreamAudioBuffer, IAudioBuffer)

StreamAudioBuffer::~StreamAudioBuffer()
{
	destroy();
}

bool StreamAudioBuffer::create(IStreamDecoder* streamDecoder)
{
	if ((m_streamDecoder = streamDecoder) != nullptr)
		return true;
	else
		return false;
}

void StreamAudioBuffer::destroy()
{
	safeDestroy(m_streamDecoder);
}

Ref< IAudioBufferCursor > StreamAudioBuffer::createCursor() const
{
	return new StreamAudioBufferCursor();
}

bool StreamAudioBuffer::getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	StreamAudioBufferCursor* ssbc = static_cast< StreamAudioBufferCursor* >(cursor);
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
