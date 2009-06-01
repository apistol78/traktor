#include <limits>
#include "Sound/XAudio2/SoundDriverXAudio2.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Math/MathUtils.h"
#include "Core/Log/Log.h"

#undef min
#undef max

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct StreamingVoiceContext : public IXAudio2VoiceCallback
{
	HANDLE m_hEvent;

	StreamingVoiceContext(HANDLE hEvent)
	:	m_hEvent(hEvent)
	{
	}

	virtual void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 BytesRequired) {}
	virtual void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() {}
	virtual void STDMETHODCALLTYPE OnStreamEnd() {}
	virtual void STDMETHODCALLTYPE OnBufferStart(void*) {}
	virtual void STDMETHODCALLTYPE OnBufferEnd( void*) { SetEvent(m_hEvent); }
	virtual void STDMETHODCALLTYPE OnLoopEnd(void*) {}   
	virtual void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT) {}
};

template < typename SampleType >
void writeSamples(void* dest, const float* samples, uint32_t samplesCount, uint32_t writeStride)
{
	SampleType* write = static_cast< SampleType* >(dest);
	for (uint32_t i = 0; i < samplesCount; ++i)
	{
		float sample = samples[i];
		sample = max(sample, -1.0f);
		sample = min(sample,  1.0f);
		write[i * writeStride] = static_cast< SampleType >(sample * std::numeric_limits< SampleType >::max());
	}
}

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.sound.SoundDriverXAudio2", SoundDriverXAudio2, SoundDriver)

SoundDriverXAudio2::SoundDriverXAudio2()
:	m_voiceCallback(0)
,	m_masteringVoice(0)
,	m_sourceVoice(0)
,	m_eventNotify(NULL)
,	m_bufferSize(0)
,	m_nextSubmitBuffer(0)
{
	std::memset(&m_wfx, 0, sizeof(m_wfx));

	m_buffers[0] =
	m_buffers[1] =
	m_buffers[2] =
	m_buffers[3] = 0;
}

SoundDriverXAudio2::~SoundDriverXAudio2()
{
	destroy();
}

bool SoundDriverXAudio2::create(const SoundDriverCreateDesc& desc)
{
	HRESULT hr;

#if !defined(_XBOX)
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

	m_eventNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!m_eventNotify)
		return false;

	UINT32 flags = 0;
#if defined(_DEBUG)
	flags |= XAUDIO2_DEBUG_ENGINE;
#endif
	hr = XAudio2Create(&m_audio.getAssign(), flags);
	if (FAILED(hr))
		return false;

	hr = m_audio->CreateMasteringVoice(&m_masteringVoice);
	if (FAILED(hr))
		return false;

	memset(&m_wfx, 0, sizeof(m_wfx));
	m_wfx.cbSize = sizeof(m_wfx);
	m_wfx.wFormatTag = WAVE_FORMAT_PCM;
	m_wfx.nChannels = desc.hwChannels;
	m_wfx.nSamplesPerSec = desc.sampleRate;
	m_wfx.wBitsPerSample = desc.bitsPerSample;
	m_wfx.nBlockAlign = m_wfx.nChannels * m_wfx.wBitsPerSample / 8;
	m_wfx.nAvgBytesPerSec = m_wfx.nSamplesPerSec * m_wfx.nBlockAlign;

	m_voiceCallback = new StreamingVoiceContext(m_eventNotify);

	hr = m_audio->CreateSourceVoice(&m_sourceVoice, &m_wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, m_voiceCallback, NULL, NULL);
	if (FAILED(hr))
		return false;

	m_sourceVoice->Start(0, 0);

	m_bufferSize = desc.frameSamples * desc.hwChannels * desc.bitsPerSample / 8;
	for (uint32_t i = 0; i < sizeof_array(m_buffers); ++i)
		m_buffers[i] = new uint8_t [m_bufferSize];

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

	m_audio.release();

	if (m_voiceCallback)
	{
		delete m_voiceCallback;
		m_voiceCallback = 0;
	}

	if (m_eventNotify)
	{
		CloseHandle(m_eventNotify);
		m_eventNotify = NULL;
	}

	for (uint32_t i = 0; i < sizeof_array(m_buffers); ++i)
	{
		delete[] m_buffers[i];
		m_buffers[i] = 0;
	}

#if !defined(_XBOX)
	CoUninitialize();
#endif
}

void SoundDriverXAudio2::wait()
{
	XAUDIO2_VOICE_STATE state;

	if (!m_sourceVoice)
		return;

	while (m_sourceVoice->GetState(&state), state.BuffersQueued >= sizeof_array(m_buffers) - 1)
		WaitForSingleObject(m_eventNotify, INFINITE);
}

void SoundDriverXAudio2::submit(const SoundBlock& soundBlock)
{
	XAUDIO2_BUFFER buffer;

	if (!m_sourceVoice)
		return;

	uint32_t blockSize = soundBlock.samplesCount * m_wfx.nChannels * m_wfx.wBitsPerSample / 8;

	// Grab buffer to submit.
	std::memset(&buffer, 0, sizeof(buffer));
	buffer.AudioBytes = std::min(m_bufferSize, blockSize);
	buffer.pAudioData = m_buffers[m_nextSubmitBuffer];
	m_nextSubmitBuffer = (m_nextSubmitBuffer + 1) % sizeof_array(m_buffers);

	// Fill buffer with samples.
	uint8_t* data = (uint8_t*)buffer.pAudioData;
	switch (m_wfx.wBitsPerSample)
	{
	case 8:
		for (uint32_t i = 0; i < soundBlock.maxChannel; ++i)
			writeSamples< uint8_t >(&data[i], soundBlock.samples[i], soundBlock.samplesCount, soundBlock.maxChannel);
		break;

	case 16:
		for (uint32_t i = 0; i < soundBlock.maxChannel; ++i)
			writeSamples< int16_t >(&data[i * 2], soundBlock.samples[i], soundBlock.samplesCount, soundBlock.maxChannel);
		break;
	}

	m_sourceVoice->SubmitSourceBuffer(&buffer);
}

	}
}
