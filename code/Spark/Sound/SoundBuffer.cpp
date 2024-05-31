/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Spark/Sound.h"
#include "Spark/Sound/SoundBuffer.h"

namespace traktor::spark
{
	namespace
	{

struct SoundBufferCursor : public RefCountImpl< sound::IAudioBufferCursor >
{
	AutoArrayPtr< float, AllocatorFree > m_samples[2];
	int32_t m_position = 0;

	SoundBufferCursor()
	{
		m_samples[0].reset((float*)getAllocator()->alloc(4096 * sizeof(float), 16, T_FILE_LINE));
		m_samples[1].reset((float*)getAllocator()->alloc(4096 * sizeof(float), 16, T_FILE_LINE));
	}

	virtual void setParameter(sound::handle_t id, float parameter) override final
	{
	}

	virtual void disableRepeat() override final
	{
	}

	virtual void reset() override final
	{
		m_position = 0;
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SoundBuffer", SoundBuffer, sound::IAudioBuffer)

SoundBuffer::SoundBuffer(const Sound* sound)
:	m_sound(sound)
{
}

Ref< sound::IAudioBufferCursor > SoundBuffer::createCursor() const
{
	return new SoundBufferCursor();
}

bool SoundBuffer::getBlock(sound::IAudioBufferCursor* cursor, const sound::IAudioMixer* mixer, sound::AudioBlock& outBlock) const
{
	SoundBufferCursor* fsbc = static_cast< SoundBufferCursor* >(cursor);
	T_ASSERT(fsbc);

	// Have we reached the end?
	const int32_t position = fsbc->m_position;
	if (position >= int32_t(m_sound->getSampleCount()))
		return false;

	// Calculate number of samples to convert for this block.
	int32_t sampleCount = m_sound->getSampleCount() - position;
	sampleCount = std::min< uint32_t >(sampleCount, outBlock.samplesCount);
	sampleCount = alignDown(sampleCount, 4);
	sampleCount = std::min< uint32_t >(sampleCount, 4096);
	if (sampleCount == 0)
		return false;

	// Convert samples into fp32 buffer.
	for (uint8_t ii = 0; ii < m_sound->getChannels(); ++ii)
	{
		const int16_t* ss = m_sound->getSamples(ii) + position;
		float* ds = fsbc->m_samples[ii].ptr();

		for (int32_t i = 0; i < sampleCount; ++i)
			*ds++ = float(*ss++) / 32767.0f;

		outBlock.samples[ii] = fsbc->m_samples[ii].ptr();
	}

	// Expand mono sounds to both channels.
	if (m_sound->getChannels() == 1)
		outBlock.samples[1] = outBlock.samples[0];

	outBlock.samplesCount = sampleCount;
	outBlock.sampleRate = m_sound->getSampleRate();
	outBlock.maxChannel = 2;

	fsbc->m_position += sampleCount;
	return true;
}

}
