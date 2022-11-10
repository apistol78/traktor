/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Sound/OpenSL/AudioDriverOpenSL.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.AudioDriverOpenSL", 0, AudioDriverOpenSL, IAudioDriver)

AudioDriverOpenSL::AudioDriverOpenSL()
:	m_engineObject(0)
,	m_engineEngine(0)
,	m_outputMixObject(0)
,	m_playerObject(0)
,	m_playerPlay(0)
,	m_playerBufferQueue(0)
{
}

bool AudioDriverOpenSL::create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer)
{
	SLresult result;

	result = slCreateEngine(&m_engineObject, 0, NULL, 0, NULL, NULL);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; slCreateEngine failed." << Endl;
		return false;
	}

	result = (*m_engineObject)->Realize(m_engineObject, SL_BOOLEAN_FALSE);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; Failed to realize engine object." << Endl;
		return false;
	}

	result = (*m_engineObject)->GetInterface(m_engineObject, SL_IID_ENGINE, &m_engineEngine);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; Failed to get engine interface." << Endl;
		return false;
	}

	SLuint32 sampleRate = 0;
	switch (desc.sampleRate)
	{
	case 22050:
		sampleRate = SL_SAMPLINGRATE_22_05;
		break;

	case 44100:
		sampleRate = SL_SAMPLINGRATE_44_1;
		break;

	case 48000:
		sampleRate = SL_SAMPLINGRATE_48;
		break;

	default:
		{
			log::error << L"Unable to create OpenSL sound driver; Unsupported sample rate " << desc.sampleRate << L"." << Endl;
			return false;
		}
	}

	const SLInterfaceID ids[] = { SL_IID_VOLUME };
	const SLboolean req[] = { SL_BOOLEAN_FALSE };
	result = (*m_engineEngine)->CreateOutputMix(m_engineEngine, &m_outputMixObject, 1, ids, req);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; Failed to create output mixer object." << Endl;
		return false;
	}

	result = (*m_outputMixObject)->Realize(m_outputMixObject, SL_BOOLEAN_FALSE);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; Failed to realize output mixer object." << Endl;
		return false;
	}

	int32_t speakers = 0;
	switch (desc.hwChannels)
	{
	case 1:
		speakers = SL_SPEAKER_FRONT_CENTER;
		break;

	case 2:
		speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
		break;

	default:
		{
			log::error << L"Unable to create OpenSL sound driver; Invalid number of hardware channels (" << desc.hwChannels << L")." << Endl;
			return false;
		}
	}

	SLDataLocator_AndroidSimpleBufferQueue locatorBufferQueue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
	SLDataFormat_PCM formatPCM =
	{
		SL_DATAFORMAT_PCM,
		(SLuint32)desc.hwChannels,
		(SLuint32)sampleRate,
		SL_PCMSAMPLEFORMAT_FIXED_16,
		SL_PCMSAMPLEFORMAT_FIXED_16,
		(SLuint32)speakers,
		SL_BYTEORDER_LITTLEENDIAN
	};

	SLDataLocator_OutputMix locatorOutputMix = { SL_DATALOCATOR_OUTPUTMIX, m_outputMixObject };

	SLDataSource audioSource = { &locatorBufferQueue, &formatPCM };
	SLDataSink audioSink = { &locatorOutputMix, NULL };

	const SLInterfaceID ids1[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
	const SLboolean req1[] = { SL_BOOLEAN_TRUE };
	result = (*m_engineEngine)->CreateAudioPlayer(m_engineEngine, &m_playerObject, &audioSource, &audioSink, 1, ids1, req1);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; Failed to create audio player object." << Endl;
		return false;
	}

	result = (*m_playerObject)->Realize(m_playerObject, SL_BOOLEAN_FALSE);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; Failed to realize audio player object." << Endl;
		return false;
	}

	result = (*m_playerObject)->GetInterface(m_playerObject, SL_IID_PLAY, &m_playerPlay);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; Failed to get player interface." << Endl;
		return false;
	}

	result = (*m_playerObject)->GetInterface(m_playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &m_playerBufferQueue);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; Failed to get buffer queue interface." << Endl;
		return false;
	}

	result = (*m_playerBufferQueue)->RegisterCallback(m_playerBufferQueue, &AudioDriverOpenSL::queueCallback, this);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; Failed to register queue callback." << Endl;
		return false;
	}

	result = (*m_playerPlay)->SetPlayState(m_playerPlay, SL_PLAYSTATE_PLAYING);
	if (result != SL_RESULT_SUCCESS)
	{
		log::error << L"Unable to create OpenSL sound driver; Failed to set playing state." << Endl;
		return false;
	}

	m_desc = desc;
	m_queueBuffer.reset(new int16_t [desc.hwChannels * desc.frameSamples]);
	m_eventQueue.broadcast();

	return true;
}

void AudioDriverOpenSL::destroy()
{
}

void AudioDriverOpenSL::wait()
{
	m_eventQueue.wait(1000);
}

void AudioDriverOpenSL::submit(const SoundBlock& soundBlock)
{
	int16_t* write = m_queueBuffer.ptr();
	for (int32_t i = 0; i < soundBlock.samplesCount; ++i)
	{
		for (int32_t channel = 0; channel < m_desc.hwChannels; ++channel)
		{
			if (soundBlock.samples[channel])
				*write++ = int16_t(clamp(soundBlock.samples[channel][i], -1.0f, 1.0f) * 32767.0f);
			else
				*write++ = 0;
		}
	}

	(*m_playerBufferQueue)->Enqueue(
		m_playerBufferQueue,
		m_queueBuffer.ptr(),
		soundBlock.samplesCount * m_desc.hwChannels * sizeof(int16_t)
	);
}

void AudioDriverOpenSL::queueCallback(
	SLAndroidSimpleBufferQueueItf caller,
	void *pContext
)
{
	AudioDriverOpenSL* this_ = reinterpret_cast< AudioDriverOpenSL* >(pContext);
	T_ASSERT (this_);

	this_->m_eventQueue.broadcast();
}

	}
}
