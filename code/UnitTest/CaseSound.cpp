/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Signal.h"
#include "Core/Timer/Timer.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/ISoundDriver.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundSystem.h"
#include "UnitTest/CaseSound.h"

namespace traktor
{
	namespace
	{

class TestSoundDriver : public RefCountImpl< sound::ISoundDriver >
{
public:
	Timer& m_timer;
	Signal& m_signal;
	double m_signalAt;

	TestSoundDriver(Timer& timer, Signal& signal)
	:	m_timer(timer)
	,	m_signal(signal)
	,	m_signalAt(0.0)
	{
	}

	virtual bool create(const SystemApplication& sysapp, const sound::SoundDriverCreateDesc& desc, Ref< sound::ISoundMixer >& outMixer) T_OVERRIDE T_FINAL
	{
		return true;
	}

	virtual void destroy() T_OVERRIDE T_FINAL
	{
	}

	virtual void wait() T_OVERRIDE T_FINAL
	{
	}

	virtual void submit(const sound::SoundBlock& soundBlock) T_OVERRIDE T_FINAL
	{
		if (m_signal.wait(0))
			return;

		for (uint32_t i = 0; i < soundBlock.samplesCount; ++i)
		{
			if (soundBlock.samples[0][i] > 1.0f)
			{
				m_signalAt = m_timer.getElapsedTime();
				m_signal.set();
			}
		}
	}
};

class TestSoundBuffer : public RefCountImpl< sound::ISoundBuffer >
{
public:
	mutable float m_block[16];

	TestSoundBuffer()
	{
		for (int i = 0; i < sizeof_array(m_block); ++i)
			m_block[i] = 2.0f;
	}

	virtual Ref< sound::ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL
	{
		return 0;
	}

	virtual bool getBlock(sound::ISoundBufferCursor* cursor, const sound::ISoundMixer* mixer, sound::SoundBlock& outBlock) const T_OVERRIDE T_FINAL
	{
		outBlock.samples[0] = m_block;
		outBlock.samplesCount = sizeof_array(m_block);
		outBlock.sampleRate = 44100;
		outBlock.maxChannel = 1;
		return true;
	}
};

	}

void CaseSound::run()
{
	Timer timer;
	Signal signal;

	TestSoundDriver soundDriver(timer, signal);
	sound::SoundSystem soundSystem(&soundDriver);

	sound::SoundSystemCreateDesc desc;
	desc.channels = 1;
	desc.driverDesc.sampleRate = 44100;
	desc.driverDesc.bitsPerSample = 16;
	desc.driverDesc.hwChannels = 1;
	desc.driverDesc.frameSamples = 1500;

	soundSystem.create(desc);

	TestSoundBuffer soundBuffer;

	timer.start();
	soundSystem.getChannel(0)->play(&soundBuffer, 0, 1.0f, 0.0f, 0.0f);

	signal.wait();

	double latency = soundDriver.m_signalAt * 1000;
	CASE_ASSERT (latency < 10);

	soundSystem.destroy();
}

}
