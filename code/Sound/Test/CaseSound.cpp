/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Test/CaseSound.h"

#include "Core/Thread/Signal.h"
#include "Core/Timer/Timer.h"
#include "Sound/AudioChannel.h"
#include "Sound/AudioSystem.h"
#include "Sound/IAudioBuffer.h"
#include "Sound/IAudioDriver.h"
#include "Sound/Sound.h"

namespace traktor::sound::test
{
namespace
{

class TestAudioDriver : public RefCountImpl< sound::IAudioDriver >
{
public:
	Timer& m_timer;
	Signal& m_signal;
	double m_signalAt;

	explicit TestAudioDriver(Timer& timer, Signal& signal)
		: m_timer(timer)
		, m_signal(signal)
		, m_signalAt(0.0)
	{
	}

	virtual bool create(const SystemApplication& sysapp, const sound::AudioDriverCreateDesc& desc, Ref< sound::IAudioMixer >& outMixer) override final
	{
		return true;
	}

	virtual void destroy() override final
	{
	}

	virtual void wait() override final
	{
	}

	virtual void submit(const sound::AudioBlock& block) override final
	{
		for (uint32_t i = 0; i < block.samplesCount; ++i)
		{
			if (block.samples[0][i] > 1.0f)
			{
				m_signalAt = m_timer.getElapsedTime();
				m_signal.set();
				break;
			}
		}
	}
};

class TestSoundBuffer : public RefCountImpl< sound::IAudioBuffer >
{
public:
	mutable float m_block[16];

	TestSoundBuffer()
	{
		for (int i = 0; i < sizeof_array(m_block); ++i)
			m_block[i] = 2.0f;
	}

	virtual Ref< sound::IAudioBufferCursor > createCursor() const override final
	{
		return nullptr;
	}

	virtual bool getBlock(sound::IAudioBufferCursor* cursor, const sound::IAudioMixer* mixer, sound::AudioBlock& outBlock) const override final
	{
		outBlock.samples[0] = m_block;
		outBlock.samplesCount = sizeof_array(m_block);
		outBlock.sampleRate = 44100;
		outBlock.maxChannel = 1;
		return true;
	}
};

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.test.CaseSound", 0, CaseSound, traktor::test::Case)

void CaseSound::run()
{
	Timer timer;
	Signal signal;

	TestAudioDriver audioDriver(timer, signal);
	sound::AudioSystem audioSystem(&audioDriver);

	sound::AudioSystemCreateDesc desc;
	desc.channels = 1;
	desc.driverDesc.sampleRate = 44100;
	desc.driverDesc.bitsPerSample = 16;
	desc.driverDesc.hwChannels = 1;
	desc.driverDesc.frameSamples = 1500;

	bool result = audioSystem.create(desc);
	CASE_ASSERT(result);
	if (!result)
		return;

	TestSoundBuffer soundBuffer;

	timer.reset();
	audioSystem.getChannel(0)->play(&soundBuffer, 0, 1.0f, false, 0);

	signal.wait();

	const double latency = audioDriver.m_signalAt * 1000;
	CASE_ASSERT(latency < 10);

	audioSystem.destroy();
}

}
