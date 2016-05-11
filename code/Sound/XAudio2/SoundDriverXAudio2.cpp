#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Core/Serialization/ISerializable.h"
#include "Sound/XAudio2/SoundDriverXAudio2.h"

#undef min
#undef max

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct EngineCallback : public IXAudio2EngineCallback
{
	HANDLE m_hEvent;
	HRESULT& m_hResult;

	EngineCallback(HANDLE hEvent, HRESULT& hResult)
	:	m_hEvent(hEvent)
	,	m_hResult(hResult)
	{
	}

	virtual void STDMETHODCALLTYPE OnProcessingPassStart() {}

	virtual void STDMETHODCALLTYPE OnProcessingPassEnd() {}

	virtual void STDMETHODCALLTYPE OnCriticalError(HRESULT hResult) { m_hResult = hResult; SetEvent(m_hEvent); }
};

struct VoiceCallback : public IXAudio2VoiceCallback
{
	HANDLE m_hEvent;
	HRESULT& m_hResult;

	VoiceCallback(HANDLE hEvent, HRESULT& hResult)
	:	m_hEvent(hEvent)
	,	m_hResult(hResult)
	{
	}

	virtual void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 BytesRequired) {}
	virtual void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() {}
	virtual void STDMETHODCALLTYPE OnStreamEnd() {}
	virtual void STDMETHODCALLTYPE OnBufferStart(void*) {}
	virtual void STDMETHODCALLTYPE OnBufferEnd( void*) { m_hResult = S_OK; SetEvent(m_hEvent); }
	virtual void STDMETHODCALLTYPE OnLoopEnd(void*) {}   
	virtual void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT hResult) { m_hResult = hResult; SetEvent(m_hEvent); }
};

template < typename SampleType >
struct CastSample
{
	SampleType cast(float sample) const
	{
		return static_cast< SampleType >(sample * std::numeric_limits< SampleType >::max());
	}
};

#if !defined(_XBOX)

template < >
struct CastSample < int8_t >
{
	__m128 f;

	CastSample()
	{
		static const float T_ALIGN16 c_int8max = std::numeric_limits< int8_t >::max();
		f = _mm_load_ss(&c_int8max);
	}

	int8_t cast(float sample) const
	{
		__m128 s = _mm_load_ss(&sample);
		__m128 sf = _mm_mul_ss(s, f);
		return (int8_t)_mm_cvtt_ss2si(sf);
	}
};

template < >
struct CastSample < int16_t >
{
	__m128 f;

	CastSample()
	{
		static const float T_ALIGN16 c_int16max = std::numeric_limits< int16_t >::max();
		f = _mm_load_ss(&c_int16max);
	}

	int16_t cast(float sample) const
	{
		__m128 s = _mm_load_ss(&sample);
		__m128 sf = _mm_mul_ss(s, f);
		return (int16_t)_mm_cvtt_ss2si(sf);
	}
};

#endif

template < typename SampleType >
void writeSamples(void* dest, const float* samples, uint32_t samplesCount, uint32_t writeStride)
{
	CastSample< SampleType > cs;
	SampleType* write = static_cast< SampleType* >(dest);
	for (uint32_t i = 0; i < samplesCount; ++i)
	{
		float sample = clamp(*samples++, -1.0f, 1.0f);
		*write = cs.cast(sample);
		write += writeStride;
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverXAudio2", 0, SoundDriverXAudio2, ISoundDriver)

SoundDriverXAudio2::SoundDriverXAudio2()
:	m_engineCallback(0)
,	m_voiceCallback(0)
,	m_masteringVoice(0)
,	m_sourceVoice(0)
,	m_eventNotify(NULL)
,	m_hResult(S_OK)
,	m_bufferSize(0)
,	m_nextSubmitBuffer(0)
{
	std::memset(&m_wfx, 0, sizeof(m_wfx));
	for (uint32_t i = 0; i < sizeof_array(m_buffers); ++i)
		m_buffers[i] = 0;
}

SoundDriverXAudio2::~SoundDriverXAudio2()
{
	destroy();
}

bool SoundDriverXAudio2::create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	m_desc = desc;

	// Allocate submission buffers.
	m_bufferSize = desc.frameSamples * desc.hwChannels * desc.bitsPerSample / 8;
	for (uint32_t i = 0; i < sizeof_array(m_buffers); ++i)
	{
		m_buffers[i] = (uint8_t*)Alloc::acquireAlign(m_bufferSize, 16, T_FILE_LINE);
		if (!m_buffers[i])
		{
			log::error << L"Unable to create XAudio2 sound driver; Out of memory" << Endl;
			return false;
		}
	}

#if !defined(_XBOX)
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

	m_eventNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!m_eventNotify)
	{
		log::error << L"Unable to create XAudio2 sound driver; CreateEvent failed" << Endl;
		return false;
	}

	// Create callback objects.
	m_engineCallback = new EngineCallback(m_eventNotify, m_hResult);
	m_voiceCallback = new VoiceCallback(m_eventNotify, m_hResult);

	if (!reset())
		return false;

	return true;
}

void SoundDriverXAudio2::destroy()
{
	if (m_sourceVoice)
	{
		m_sourceVoice->Stop(0);
		m_sourceVoice->DestroyVoice();
		m_sourceVoice = 0;
	}

	if (m_masteringVoice)
	{
		m_masteringVoice->DestroyVoice();
		m_masteringVoice = 0;
	}

	if (m_audio)
	{
		m_audio->UnregisterForCallbacks(m_engineCallback);
		m_audio.release();
	}

	if (m_voiceCallback)
	{
		delete m_voiceCallback;
		m_voiceCallback = 0;
	}

	if (m_engineCallback)
	{
		delete m_engineCallback;
		m_engineCallback = 0;
	}

	if (m_eventNotify)
	{
		CloseHandle(m_eventNotify);
		m_eventNotify = NULL;
	}

	for (uint32_t i = 0; i < sizeof_array(m_buffers); ++i)
	{
		Alloc::freeAlign(m_buffers[i]);
		m_buffers[i] = 0;
	}

#if !defined(_XBOX)
	CoUninitialize();
#endif
}

void SoundDriverXAudio2::wait()
{
	XAUDIO2_VOICE_STATE state;

	if (!m_sourceVoice || FAILED(m_hResult))
		return;

	while (m_sourceVoice->GetState(&state), state.BuffersQueued >= 3)
	{
		WaitForSingleObject(m_eventNotify, INFINITE);
		if (FAILED(m_hResult))
			break;
	}
}

void SoundDriverXAudio2::submit(const SoundBlock& soundBlock)
{
	XAUDIO2_BUFFER buffer = { 0 };

	if (FAILED(m_hResult))
	{
		if (!reset())
			return;
	}

	if (!m_sourceVoice)
		return;

	uint32_t blockSize = soundBlock.samplesCount * m_wfx.Format.nChannels * m_wfx.Format.wBitsPerSample / 8;
	uint32_t channels = min< uint32_t >(m_desc.hwChannels, soundBlock.maxChannel);

	// Grab buffer to submit.
	buffer.AudioBytes = std::min(m_bufferSize, blockSize);
	buffer.pAudioData = m_buffers[m_nextSubmitBuffer];
	m_nextSubmitBuffer = (m_nextSubmitBuffer + 1) % sizeof_array(m_buffers);

	// Fill buffer with samples.
	uint8_t* data = (uint8_t*)buffer.pAudioData;
	switch (m_wfx.Format.wBitsPerSample)
	{
	case 8:
		for (uint32_t i = 0; i < channels; ++i)
		{
			T_ASSERT (soundBlock.samples[i]);
			writeSamples< int8_t >(&data[i], soundBlock.samples[i], soundBlock.samplesCount, m_desc.hwChannels);
		}
		break;

	case 16:
		for (uint32_t i = 0; i < channels; ++i)
		{
			T_ASSERT (soundBlock.samples[i]);
			writeSamples< int16_t >(&data[i * 2], soundBlock.samples[i], soundBlock.samplesCount, m_desc.hwChannels);
		}
		break;
	}

	m_sourceVoice->SubmitSourceBuffer(&buffer);
}

bool SoundDriverXAudio2::reset()
{
	HRESULT hr;

	if (m_sourceVoice)
	{
		m_sourceVoice->Stop(0);
		m_sourceVoice->DestroyVoice();
		m_sourceVoice = 0;
	}

	if (m_masteringVoice)
	{
		m_masteringVoice->DestroyVoice();
		m_masteringVoice = 0;
	}

	if (m_audio)
	{
		m_audio->UnregisterForCallbacks(m_engineCallback);
		m_audio.release();
	}

	UINT32 flags = 0;
#if defined(_DEBUG)
	flags |= XAUDIO2_DEBUG_ENGINE;
#endif
	hr = XAudio2Create(&m_audio.getAssign(), flags, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		log::error << L"Unable to create XAudio2 sound driver; XAudio2Create failed (" << int32_t(hr) << L")" << Endl;
		return false;
	}

	m_audio->RegisterForCallbacks(m_engineCallback);

	hr = m_audio->CreateMasteringVoice(
		&m_masteringVoice,
		XAUDIO2_DEFAULT_CHANNELS,
		XAUDIO2_DEFAULT_SAMPLERATE,
		0,
		0,
		NULL
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create XAudio2 sound driver; CreateMasteringVoice failed (" << m_desc.hwChannels << L" channels, " << int32_t(hr) << L")" << Endl;
		return false;
	}

	m_masteringVoice->SetVolume(1.0f);

	std::memset(&m_wfx, 0, sizeof(m_wfx));
	m_wfx.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE);
	m_wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	m_wfx.Format.nChannels = m_desc.hwChannels;
	m_wfx.Format.nSamplesPerSec = m_desc.sampleRate;
	m_wfx.Format.wBitsPerSample = m_desc.bitsPerSample;
	m_wfx.Format.nBlockAlign = m_desc.hwChannels * m_desc.bitsPerSample / 8;
	m_wfx.Format.nAvgBytesPerSec = m_desc.sampleRate * m_wfx.Format.nBlockAlign;
	m_wfx.Samples.wValidBitsPerSample = m_desc.bitsPerSample;

	switch (m_desc.hwChannels)
	{
	case 7+1:
		m_wfx.dwChannelMask |= SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
	case 5+1:
		m_wfx.dwChannelMask |= SPEAKER_FRONT_CENTER;
	case 4+1:
		m_wfx.dwChannelMask |= SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
	case 2+1:
		m_wfx.dwChannelMask |= SPEAKER_LOW_FREQUENCY;
	case 2:
		m_wfx.dwChannelMask |= SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
		break;
	default:
		log::error << L"Unable to create XAudio2 sound driver; Incorrect number of channels" << Endl;
		return false;
	}

	m_wfx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	hr = m_audio->CreateSourceVoice(
		&m_sourceVoice,
		(WAVEFORMATEX*)&m_wfx,
		0,
		XAUDIO2_DEFAULT_FREQ_RATIO,
		m_voiceCallback,
		NULL,
		NULL
	);
	if (FAILED(hr))
	{
		// Unable to create source voice with extensible wave format; try with default.
		log::warning << L"CreateSourceVoice failed (" << int32_t(hr) << L"); trying without extensible wave format..." << Endl;

		std::memset(&m_wfx, 0, sizeof(m_wfx));
		m_wfx.Format.cbSize = sizeof(WAVEFORMATEX);
		m_wfx.Format.wFormatTag = WAVE_FORMAT_PCM;
		m_wfx.Format.nChannels = m_desc.hwChannels;
		m_wfx.Format.nSamplesPerSec = m_desc.sampleRate;
		m_wfx.Format.wBitsPerSample = m_desc.bitsPerSample;
		m_wfx.Format.nBlockAlign = m_desc.hwChannels * m_desc.bitsPerSample / 8;
		m_wfx.Format.nAvgBytesPerSec = m_desc.sampleRate * m_wfx.Format.nBlockAlign;
		m_wfx.Samples.wValidBitsPerSample = m_desc.bitsPerSample;
		m_wfx.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
		m_wfx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

		hr = m_audio->CreateSourceVoice(
			&m_sourceVoice,
			(WAVEFORMATEX*)&m_wfx,
			0,
			XAUDIO2_DEFAULT_FREQ_RATIO,
			m_voiceCallback,
			NULL,
			NULL
		);
		if (FAILED(hr))
		{
			log::error << L"Unable to create XAudio2 sound driver; CreateSourceVoice failed (" << int32_t(hr) << L")" << Endl;
			return false;
		}
	}

	m_sourceVoice->SetVolume(1.0f);
	m_sourceVoice->Start(0);

	m_hResult = S_OK;
	return true;
}

	}
}
