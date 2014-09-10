#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Serialization/ISerializable.h"
#include "Sound/Ds8/SoundDriverDs8.h"

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
		float sample = clamp(samples[i], -1.0f, 1.0f);
		write[i * writeStride] = SampleTypeTransform::transform(sample);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverDs8", 0, SoundDriverDs8, ISoundDriver)

bool SoundDriverDs8::create(void* nativeHandle, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	DSBUFFERDESC dsbd;
	HRESULT hr;

	hr = DirectSoundCreate(NULL, (IDirectSound**)&m_ds.getAssign(), NULL);
	if (FAILED(hr))
	{
		log::error << L" Failed to create DS8 driver, unable create DirectSound object" << Endl;
		return false;
	}

	hr = m_ds->SetCooperativeLevel(
		GetDesktopWindow(),
		DSSCL_PRIORITY
	);
	if (FAILED(hr))
	{
		log::error << L"Failed to create DS8 driver, unable to set cooperative level" << Endl;
		return false;
	}

	std::memset(&m_wfx, 0, sizeof(m_wfx));
	m_wfx.cbSize = sizeof(m_wfx);
	m_wfx.wFormatTag = WAVE_FORMAT_PCM;
	m_wfx.nChannels = desc.hwChannels;
	m_wfx.nSamplesPerSec = desc.sampleRate;
	m_wfx.wBitsPerSample = desc.bitsPerSample;
	m_wfx.nBlockAlign = WORD(m_wfx.nChannels * m_wfx.wBitsPerSample / 8);
	m_wfx.nAvgBytesPerSec = m_wfx.nSamplesPerSec * m_wfx.nBlockAlign;

	m_frameSamples = desc.frameSamples;
	m_bufferSize = (desc.frameSamples * 3) * desc.hwChannels * desc.bitsPerSample / 8;
	m_bufferWrite = 0;

	std::memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
	dsbd.dwBufferBytes = m_bufferSize;
	dsbd.lpwfxFormat = &m_wfx;

	hr = m_ds->CreateSoundBuffer(
		&dsbd,
		&m_dsBuffer.getAssign(),
		NULL
	);
	if (FAILED(hr))
	{
		log::error << L"Failed to create DS8 driver, unable to create sound buffer" << Endl;
		return false;
	}

	hr = m_dsBuffer->SetCurrentPosition(0);
	if (FAILED(hr))
	{
		log::error << L"Failed to create DS8 driver, unable to reset sound buffer position" << Endl;
		return false;
	}

	LPVOID ptr[2];
	DWORD size[2];

	hr = m_dsBuffer->Lock(0, m_bufferSize, &ptr[0], &size[0], &ptr[1], &size[1], 0);
	if (FAILED(hr))
	{
		log::error << L"Failed to create DS8 driver, unable to lock sound buffer" << Endl;
		return false;
	}

	T_ASSERT (size[1] == 0);
	std::memset(ptr[0], 0, size[0]);

	hr = m_dsBuffer->Unlock(ptr[0], size[0], ptr[1], size[1]);
	if (FAILED(hr))
	{
		log::error << L"Failed to create DS8 driver, unable to unlock sound buffer" << Endl;
		return false;
	}

	// Setup notification positions.
	hr = m_dsBuffer->QueryInterface(IID_IDirectSoundNotify8, (void**)&m_dsNotify.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Failed to create DS8 driver, unable to acquire notification instance" << Endl;
		return false;
	}

	m_eventNotify[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_eventNotify[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_eventNotify[2] = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_dsbpn[0].dwOffset = (m_bufferSize / 6);
	m_dsbpn[0].hEventNotify = m_eventNotify[0];
	m_dsbpn[1].dwOffset = (m_bufferSize / 6) + (m_bufferSize * 1) / 3;
	m_dsbpn[1].hEventNotify = m_eventNotify[1];
	m_dsbpn[2].dwOffset = (m_bufferSize / 6) + (m_bufferSize * 2) / 3;
	m_dsbpn[2].hEventNotify = m_eventNotify[2];

	hr = m_dsNotify->SetNotificationPositions(3, m_dsbpn);
	if (FAILED(hr))
	{
		log::error << L"Failed to create DS8 driver, unable to set notification positions" << Endl;
		return false;
	}

	return true;
}

void SoundDriverDs8::destroy()
{
	m_dsNotify.release();
	m_dsBuffer.release();
	m_ds.release();
}

void SoundDriverDs8::wait()
{
	DWORD result = WaitForMultipleObjects(3, m_eventNotify, FALSE, INFINITE);
	if (result == WAIT_OBJECT_0)
		m_bufferWrite = (m_bufferSize * 2) / 3;
	else if (result == WAIT_OBJECT_0 + 1)
		m_bufferWrite = (m_bufferSize * 0) / 3;
	else if (result == WAIT_OBJECT_0 + 2)
		m_bufferWrite = (m_bufferSize * 1) / 3;
}

void SoundDriverDs8::submit(const SoundBlock& soundBlock)
{
	uint8_t* ptr[2];
	DWORD size[2];
	DWORD status;
	HRESULT hr;

	T_ASSERT (m_frameSamples == soundBlock.samplesCount);
	T_ASSERT (m_wfx.nChannels == soundBlock.maxChannel);

	uint32_t sampleSize = m_wfx.wBitsPerSample >> 3;
	uint32_t blockSize = soundBlock.samplesCount * m_wfx.nChannels * sampleSize;

	// Lock writable frame.
	hr = m_dsBuffer->Lock(
		m_bufferWrite,
		blockSize,
		(void**)&ptr[0],
		&size[0],
		(void**)&ptr[1],
		&size[1],
		0
	);
	if (FAILED(hr))
	{
		log::error << L"Submit failed, unable to lock sound buffer" << Endl;
		return;
	}
	T_ASSERT (!size[1]);

	// Write sound block into locked frame.
	switch (sampleSize)
	{
	case 1:
		{
			for (uint32_t i = 0; i < m_wfx.nChannels; ++i)
				writeSamples< uint8_t, UnsignedType< uint8_t > >(ptr[0] + sampleSize * i, soundBlock.samples[i], soundBlock.samplesCount, m_wfx.nChannels);
		}
		break;

	case 2:
		{
			for (uint32_t i = 0; i < m_wfx.nChannels; ++i)
				writeSamples< int16_t, SignedType< int16_t > >(ptr[0] + sampleSize * i, soundBlock.samples[i], soundBlock.samplesCount, m_wfx.nChannels);
		}
		break;

	case 4:
		{
			for (uint32_t i = 0; i < m_wfx.nChannels; ++i)
				writeSamples< int32_t, SignedType< int32_t > >(ptr[0] + sampleSize * i, soundBlock.samples[i], soundBlock.samplesCount, m_wfx.nChannels);
		}
		break;

	default:
		T_FATAL_ERROR;
		break;
	}	

	// Unlock frame.
	hr = m_dsBuffer->Unlock(ptr[0], size[0], ptr[1], size[1]);
	if (FAILED(hr))
	{
		log::error << L"Submit failed, unable to unlock sound buffer" << Endl;
		return;
	}

	// Start playing buffer if it hasn't already.
	hr = m_dsBuffer->GetStatus(&status);
	if (FAILED(hr))
	{
		log::error << L"Submit failed, unable to get status" << Endl;
		return;
	}

	if (!(status & DSBSTATUS_PLAYING))
	{
		hr = m_dsBuffer->Play(0, 0, DSBPLAY_LOOPING);
		if (FAILED(hr))
			log::error << L"Submit failed, unable to start playing" << Endl;
	}
}

	}
}
