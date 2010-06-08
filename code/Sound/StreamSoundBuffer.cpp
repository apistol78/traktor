#include "Core/Log/Log.h"
#include "Sound/IStreamDecoder.h"
#include "Sound/StreamSoundBuffer.h"

namespace traktor
{
	namespace sound
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

	virtual void reset()
	{
		m_position = 0;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.StreamSoundBuffer", StreamSoundBuffer, ISoundBuffer)

StreamSoundBuffer::StreamSoundBuffer()
:	m_position(0)
{
}

bool StreamSoundBuffer::create(IStreamDecoder* streamDecoder)
{
	if ((m_streamDecoder = streamDecoder) != 0)
		return true;
	else
		return false;
}

void StreamSoundBuffer::destroy()
{
	m_streamDecoder = 0;
}

Ref< ISoundBufferCursor > StreamSoundBuffer::createCursor() const
{
	return new StreamSoundBufferCursor();
}

bool StreamSoundBuffer::getBlock(const ISoundMixer* mixer, ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	StreamSoundBufferCursor* ssbc = static_cast< StreamSoundBufferCursor* >(cursor);
	uint64_t position = ssbc->m_position;

	if (m_position > position)
	{
		log::debug << L"Rewind stream sound decoder" << Endl;
		m_streamDecoder->rewind();
		m_position = 0;
	}

	uint32_t samplesCount = outBlock.samplesCount;
	while (m_position < position)
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
}
