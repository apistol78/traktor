/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#define _WIN32_DCOM
#include <windows.h>
#include <xaudio2.h>
#include "Core/Misc/ComRef.h"
#include "Sound/IAudioDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_XAUDIO2_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*!
 * \ingroup XA2
 */
class T_DLLCLASS AudioDriverXAudio2 : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	AudioDriverXAudio2();

	virtual ~AudioDriverXAudio2();

	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const AudioBlock& soundBlock) override final;

private:
	AudioDriverCreateDesc m_desc;
	ComRef< IXAudio2 > m_audio;
	IXAudio2EngineCallback* m_engineCallback;
	IXAudio2VoiceCallback* m_voiceCallback;
	IXAudio2MasteringVoice* m_masteringVoice;
	IXAudio2SourceVoice* m_sourceVoice;
	WAVEFORMATEXTENSIBLE m_wfx;
	HANDLE m_eventNotify;
	HRESULT m_hResult;
	uint32_t m_bufferSize;
	uint8_t* m_buffers[3];
	uint32_t m_nextSubmitBuffer;

	bool reset();
};

}
