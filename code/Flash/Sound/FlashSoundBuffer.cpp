/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Misc/Align.h"
#include "Core/Misc/AutoPtr.h"
#include "Flash/FlashSound.h"
#include "Flash/Sound/FlashSoundBuffer.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

struct FlashSoundBufferCursor : public RefCountImpl< sound::ISoundBufferCursor >
{
	int32_t m_position;
	AutoArrayPtr< float, AllocatorFree > m_samples[2];

	FlashSoundBufferCursor()
	:	m_position(0)
	{
		m_samples[0].reset((float*)getAllocator()->alloc(4096 * sizeof(float), 16, T_FILE_LINE));
		m_samples[1].reset((float*)getAllocator()->alloc(4096 * sizeof(float), 16, T_FILE_LINE));
	}

	virtual void setParameter(sound::handle_t id, float parameter) T_OVERRIDE T_FINAL
	{
	}

	virtual void disableRepeat() T_OVERRIDE T_FINAL
	{
	}

	virtual void reset() T_OVERRIDE T_FINAL
	{
		m_position = 0;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashSoundBuffer", FlashSoundBuffer, sound::ISoundBuffer)

FlashSoundBuffer::FlashSoundBuffer(const FlashSound* sound)
:	m_sound(sound)
{
}

Ref< sound::ISoundBufferCursor > FlashSoundBuffer::createCursor() const
{
	return new FlashSoundBufferCursor();
}

bool FlashSoundBuffer::getBlock(sound::ISoundBufferCursor* cursor, const sound::ISoundMixer* mixer, sound::SoundBlock& outBlock) const
{
	FlashSoundBufferCursor* fsbc = static_cast< FlashSoundBufferCursor* >(cursor);
	T_ASSERT (fsbc);

	// Have we reached the end?
	int32_t position = fsbc->m_position;
	if (position >= int32_t(m_sound->getSamplesCount()))
		return false;

	// Calculate number of samples to convert for this block.
	int32_t samplesCount = m_sound->getSamplesCount() - position;
	samplesCount = std::min< uint32_t >(samplesCount, outBlock.samplesCount);
	samplesCount = alignDown(samplesCount, 4);
	samplesCount = std::min< uint32_t >(samplesCount, 4096);
	
	if (samplesCount == 0)
		return false;

	// Convert samples into fp32 buffer.
	for (uint8_t ii = 0; ii < m_sound->getChannels(); ++ii)
	{
		const int16_t* ss = m_sound->getSamples(ii) + position;
		float* ds = fsbc->m_samples[ii].ptr();

		for (int32_t i = 0; i < samplesCount; ++i)
			*ds++ = float(*ss++) / 32767.0f;

		outBlock.samples[ii] = fsbc->m_samples[ii].ptr();
	}

	// Expand mono sounds to both channels.
	if (m_sound->getChannels() == 1)
		outBlock.samples[1] = outBlock.samples[0];

	outBlock.samplesCount = samplesCount;
	outBlock.sampleRate = m_sound->getSampleRate();
	outBlock.maxChannel = 2;

	fsbc->m_position += samplesCount;
	return true;
}

	}
}
