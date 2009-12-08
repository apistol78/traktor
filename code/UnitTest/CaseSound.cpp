#include "UnitTest/CaseSound.h"
#include "Sound/ISoundDriver.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/SoundSystem.h"
#include "Sound/Sound.h"
#include "Core/Timer/Timer.h"
#include "Core/Thread/Signal.h"

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

	virtual bool create(const sound::SoundDriverCreateDesc& desc)
	{
		return true;
	}

	virtual void destroy()
	{
	}

	virtual void wait()
	{
	}

	virtual void submit(const sound::SoundBlock& soundBlock)
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
	float m_block[16];

	TestSoundBuffer()
	{
		for (int i = 0; i < sizeof_array(m_block); ++i)
			m_block[i] = 2.0f;
	}

	virtual void reset()
	{
	}

	virtual bool getBlock(double time, sound::SoundBlock& outBlock)
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
	desc.channels = 4;
	desc.driverDesc.sampleRate = 44100;
	desc.driverDesc.bitsPerSample = 16;
	desc.driverDesc.hwChannels = 1;
	desc.driverDesc.frameSamples = 1500;

	soundSystem.create(desc);

	TestSoundBuffer soundBuffer;
	sound::Sound sound(&soundBuffer);

	timer.start();
	soundSystem.play(&sound, false);

	signal.wait();

	double latency = soundDriver.m_signalAt * 1000;
	CASE_ASSERT (latency < 10);

	soundSystem.destroy();
}

}
