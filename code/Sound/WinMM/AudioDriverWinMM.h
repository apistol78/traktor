/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <windows.h>
#include "Sound/IAudioDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_WINMM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*!
 * \ingroup WMM
 */
class T_DLLCLASS AudioDriverWinMM : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	AudioDriverWinMM();

	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const SoundBlock& soundBlock) override final;

private:
	HWAVEOUT m_wo;
	WAVEFORMATEX m_wfx;
	HANDLE m_eventNotify;
	uint8_t* m_buffer;
	WAVEHDR m_blocks[3];
	uint32_t m_nextPrepareBlock;
};

	}
}

