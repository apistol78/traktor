#include "Sound/StreamSoundBuffer.h"
#include "Sound/IStreamDecoder.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace sound
	{

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

void StreamSoundBuffer::reset()
{
	m_streamDecoder->rewind();
	m_time = 0.0;
}

bool StreamSoundBuffer::getBlock(double time, SoundBlock& outBlock)
{
	T_ASSERT (m_streamDecoder);
	
	if (m_time > time)
	{
		log::warning << L"Rewinding stream; severe performance penalty" << Endl;
		reset();
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
