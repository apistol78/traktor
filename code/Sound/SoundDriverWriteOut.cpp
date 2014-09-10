#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
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

SoundDriverWriteOut::SoundDriverWriteOut(ISoundDriver* childDriver)
:	m_childDriver(childDriver)
,	m_peek(0.0f)
,	m_wait(true)
{
}

bool SoundDriverWriteOut::create(void* nativeHandle, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	if (m_childDriver)
	{
		if (!m_childDriver->create(nativeHandle, desc, outMixer))
			return false;
	}

	StringOutputStream ss;
	ss << L"swo-" << desc.sampleRate << L"-" << desc.hwChannels << L"-32fp.raw";

	m_stream = FileSystem::getInstance().open(ss.str(), File::FmWrite);
	if (!m_stream)
		return false;

	m_interleaved.reset(new float [desc.frameSamples * desc.hwChannels]);
	m_desc = desc;
	m_wait = true;

	return true;
}

void SoundDriverWriteOut::destroy()
{
	if (m_stream)
	{
		m_stream->close();
		m_stream = 0;
	}

	safeDestroy(m_childDriver);
}

void SoundDriverWriteOut::wait()
{
	if (m_childDriver)
		m_childDriver->wait();
	else
	{
		int32_t ms = int32_t(m_desc.frameSamples * 1000L / m_desc.sampleRate);
		ThreadManager::getInstance().getCurrentThread()->sleep(ms);
	}
}

void SoundDriverWriteOut::submit(const SoundBlock& soundBlock)
{
	// Submit to child driver first.
	if (m_childDriver)
		m_childDriver->submit(soundBlock);

	// Wait for first non-mute block before we start writing out
	// in order to prevent big empty files.
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

	// Shuffle samples into an interleaved format.
	float* sp = m_interleaved.ptr();
	T_ASSERT (sp);

	for (uint32_t i = 0; i < soundBlock.samplesCount; ++i)
	{
		for (uint32_t j = 0; j < m_desc.hwChannels; ++j)
			*sp++ = soundBlock.samples[j] ? soundBlock.samples[j][i] : 0.0f;
	}

	// Write out interleaved block.
	m_stream->write(
		m_interleaved.c_ptr(),
		soundBlock.samplesCount * m_desc.hwChannels * sizeof(float)
	);
}

	}
}
