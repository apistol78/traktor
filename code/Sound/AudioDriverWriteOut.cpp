/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Sound/AudioDriverWriteOut.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.AudioDriverWriteOut", 0, AudioDriverWriteOut, IAudioDriver)

AudioDriverWriteOut::AudioDriverWriteOut(IAudioDriver* childDriver)
:	m_childDriver(childDriver)
,	m_wait(true)
{
}

bool AudioDriverWriteOut::create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer)
{
	if (m_childDriver)
	{
		if (!m_childDriver->create(sysapp, desc, outMixer))
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

void AudioDriverWriteOut::destroy()
{
	if (m_stream)
	{
		m_stream->close();
		m_stream = 0;
	}

	safeDestroy(m_childDriver);
}

void AudioDriverWriteOut::wait()
{
	if (m_childDriver)
		m_childDriver->wait();
	else
	{
		int32_t ms = int32_t(m_desc.frameSamples * 1000L / m_desc.sampleRate);
		ThreadManager::getInstance().getCurrentThread()->sleep(ms);
	}
}

void AudioDriverWriteOut::submit(const SoundBlock& soundBlock)
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
	T_ASSERT(sp);

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
