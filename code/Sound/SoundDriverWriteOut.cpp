#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Sound/SoundDriverWriteOut.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverWriteOut", 0, SoundDriverWriteOut, ISoundDriver)

SoundDriverWriteOut::SoundDriverWriteOut()
:	m_peek(0.0f)
,	m_wait(true)
{
}

bool SoundDriverWriteOut::create(const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	for (uint32_t i = 0; i < desc.hwChannels; ++i)
	{
		Ref< IStream > stream = FileSystem::getInstance().open(L"Sound" + toString(i) + L".raw", File::FmWrite);
		if (!stream)
			return false;

		m_streams[i] = stream;
	}

	m_desc = desc;
	m_wait = true;

	return true;
}

void SoundDriverWriteOut::destroy()
{
	for (uint32_t i = 0; i < sizeof_array(m_streams); ++i)
	{
		if (m_streams[i])
		{
			m_streams[i]->close();
			m_streams[i] = 0;
		}
	}
}

void SoundDriverWriteOut::wait()
{
	long ms = m_desc.frameSamples * 1000L / m_desc.sampleRate;
	ThreadManager::getInstance().getCurrentThread()->sleep(ms);
}

void SoundDriverWriteOut::submit(const SoundBlock& soundBlock)
{
	// Wait for first non-mute block.
	if (m_wait)
	{
		for (uint32_t i = 0; i < m_desc.hwChannels && m_wait; ++i)
		{
			for (uint32_t j = 0; j < soundBlock.samplesCount && m_wait; ++j)
			{
				if (abs(soundBlock.samples[i][j]) >= FUZZY_EPSILON)
					m_wait = false;
			}
		}
		if (m_wait)
			return;
	}

	for (uint32_t i = 0; i < m_desc.hwChannels; ++i)
	{
		if (soundBlock.samples[i] && m_streams[i])
		{
			m_streams[i]->write(
				soundBlock.samples[i],
				soundBlock.samplesCount * sizeof(float)
			);

			for (uint32_t j = 0; j < soundBlock.samplesCount; ++j)
				m_peek = max(m_peek, soundBlock.samples[i][j]);
		}
	}
}

	}
}
