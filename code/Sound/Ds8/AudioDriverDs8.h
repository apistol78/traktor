/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/Ds8/Platform.h"
#include "Sound/IAudioDriver.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_DS8_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*!
 * \ingroup DS8
 */
class T_DLLCLASS AudioDriverDs8 : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const AudioBlock& block) override final;

private:
	ComRef< IDirectSound8 > m_ds;
	ComRef< IDirectSoundBuffer > m_dsBuffer;
	ComRef< IDirectSoundNotify8 > m_dsNotify;
	uint32_t m_frameSamples;
	uint32_t m_bufferSize;
	uint32_t m_bufferWrite;
	WAVEFORMATEX m_wfx;
	HANDLE m_eventNotify[3];
	DSBPOSITIONNOTIFY m_dsbpn[3];
};

}
