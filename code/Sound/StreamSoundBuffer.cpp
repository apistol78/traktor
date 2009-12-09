#include "Sound/StreamSoundBuffer.h"
#include "Sound/IStreamDecoder.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct StreamSoundBufferCursor : public RefCountImpl< ISoundBufferCursor >
{
	double m_time;

	StreamSoundBufferCursor()
	:	m_time(0.0)
	{
	}

	virtual void setCursor(double time)
	{
		m_time = time;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.StreamSoundBuffer", StreamSoundBuffer, ISoundBuffer)

StreamSoundBuffer::StreamSoundBuffer()
:	m_time(0.0)
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

bool StreamSoundBuffer::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	double time = static_cast< StreamSoundBufferCursor* >(cursor)->m_time;
	
	if (m_time > time)
	{
		m_streamDecoder->rewind();
		m_time = 0.0;
	}

	while (m_time < time)
	{
		if (!m_streamDecoder->getBlock(outBlock))
			return false;

		m_time += double(outBlock.samplesCount) / outBlock.sampleRate;
	}

	return true;
}

	}
}
