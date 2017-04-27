/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Serialization/ISerializable.h"
#include "Sound/WinMM/SoundDriverWinMM.h"

#undef min
#undef max

namespace traktor
{
	namespace sound
	{
		namespace
		{

template < typename SampleType >
struct UnsignedType
{
	static SampleType transform(float sample)
	{
		return static_cast< SampleType >((sample * 0.5f + 0.5f) * std::numeric_limits< SampleType >::max());
	}
};

template < typename SampleType >
struct SignedType
{
	static SampleType transform(float sample)
	{
		return static_cast< SampleType >(sample * std::numeric_limits< SampleType >::max());
	}
};

template < typename SampleType, typename SampleTypeTransform >
void writeSamples(void* dest, const float* samples, uint32_t samplesCount, uint32_t writeStride)
{
	SampleType* write = static_cast< SampleType* >(dest);
	for (uint32_t i = 0; i < samplesCount; ++i)
	{
		float sample = samples[i];
		sample = max(sample, -1.0f);
		sample = min(sample,  1.0f);
		write[i * writeStride] = SampleTypeTransform::transform(sample);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverWinMM", 0, SoundDriverWinMM, ISoundDriver)

SoundDriverWinMM::SoundDriverWinMM()
:	m_wo(NULL)
,	m_eventNotify(NULL)
,	m_buffer(0)
,	m_nextPrepareBlock(0)
{
}

bool SoundDriverWinMM::create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	m_eventNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!m_eventNotify)
		return false;

	memset(&m_wfx, 0, sizeof(m_wfx));
	m_wfx.cbSize = sizeof(m_wfx);
	m_wfx.wFormatTag = WAVE_FORMAT_PCM;
	m_wfx.nChannels = desc.hwChannels;
	m_wfx.nSamplesPerSec = desc.sampleRate;
	m_wfx.wBitsPerSample = desc.bitsPerSample;
	m_wfx.nBlockAlign = m_wfx.nChannels * m_wfx.wBitsPerSample / 8;
	m_wfx.nAvgBytesPerSec = m_wfx.nSamplesPerSec * m_wfx.nBlockAlign;

	MMRESULT result = waveOutOpen(
		&m_wo,
		WAVE_MAPPER,
		&m_wfx,
		(DWORD_PTR)m_eventNotify,
		NULL,
		CALLBACK_EVENT
	);
	if (result != MMSYSERR_NOERROR)
		return false;

	uint32_t frameSize = desc.hwChannels * desc.frameSamples * desc.bitsPerSample / 8;

	m_buffer = (uint8_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, frameSize * sizeof_array(m_blocks));
	if (!m_buffer)
		return false;

	memset(m_blocks, 0, sizeof(m_blocks));
	for (int i = 0; i < sizeof_array(m_blocks); ++i)
	{
		m_blocks[i].dwBufferLength = frameSize;
		m_blocks[i].lpData = (LPSTR)&m_buffer[i * frameSize];
		m_blocks[i].dwFlags = WHDR_DONE;
	}

	return true;
}

void SoundDriverWinMM::destroy()
{
	if (m_wo)
	{
		waveOutClose(m_wo);
		m_wo = NULL;
	}
	if (m_buffer)
	{
		HeapFree(GetProcessHeap(), 0, m_buffer);
		m_buffer = 0;
	}
	if (m_eventNotify)
	{
		DeleteObject(m_eventNotify);
		m_eventNotify = NULL;
	}
}

void SoundDriverWinMM::wait()
{
	WAVEHDR* block = &m_blocks[m_nextPrepareBlock];
	while ((block->dwFlags & WHDR_DONE) == 0)
		WaitForSingleObject(m_eventNotify, INFINITE);
}

void SoundDriverWinMM::submit(const SoundBlock& soundBlock)
{
	// Grab block to prepare.
	WAVEHDR* block = &m_blocks[m_nextPrepareBlock];
	m_nextPrepareBlock = (m_nextPrepareBlock + 1) % 3;

	// Ensure block isn't prepared.
	waveOutUnprepareHeader(m_wo, block, sizeof(WAVEHDR));

	// Fill block's buffer with samples.
	uint8_t* data = (uint8_t*)block->lpData;
	uint32_t sampleSize = m_wfx.wBitsPerSample >> 3;

	switch (sampleSize)
	{
	case 1:
		for (uint32_t i = 0; i < m_wfx.nChannels; ++i)
			writeSamples< uint8_t, UnsignedType< uint8_t > >(data + 1 * i, soundBlock.samples[i], soundBlock.samplesCount, m_wfx.nChannels);
		break;
	case 2:
		for (uint32_t i = 0; i < m_wfx.nChannels; ++i)
			writeSamples< int16_t, SignedType< int16_t > >(data + 2 * i, soundBlock.samples[i], soundBlock.samplesCount, m_wfx.nChannels);
		break;
	case 4:
		for (uint32_t i = 0; i < m_wfx.nChannels; ++i)
			writeSamples< int32_t, SignedType< int32_t > >(data + 4 * i, soundBlock.samples[i], soundBlock.samplesCount, m_wfx.nChannels);
		break;
	}

	// Prepare block and enqueue for playback.
	waveOutPrepareHeader(m_wo, block, sizeof(WAVEHDR));
	waveOutWrite(m_wo, block, sizeof(WAVEHDR));
}

	}
}
