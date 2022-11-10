/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/IAudioDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_ALSA_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct _snd_pcm;
struct _snd_pcm_hw_params;
struct _snd_pcm_sw_params;

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS AudioDriverAlsa : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	AudioDriverAlsa();

	virtual ~AudioDriverAlsa();

	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const SoundBlock& soundBlock) override final;

private:
	AudioDriverCreateDesc m_desc;
	_snd_pcm* m_handle;
	_snd_pcm_hw_params* m_hw_params;
	_snd_pcm_sw_params* m_sw_params;
	int16_t* m_buffer;
	bool m_started;
};

	}
}

