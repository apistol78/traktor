/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#if defined(__APPLE__)
#   include <OpenAL/al.h>
#   include <OpenAL/alc.h>
#else
#   include <AL/al.h>
#   include <AL/alc.h>
#endif
#include "Core/Misc/AutoPtr.h"
#include "Sound/IAudioDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_OPENAL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*!
 * \ingroup OpenAL
 */
class T_DLLCLASS AudioDriverOpenAL : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	AudioDriverOpenAL();

	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const AudioBlock& block) override final;

private:
	ALCdevice* m_device;
	ALCcontext* m_context;
	AudioDriverCreateDesc m_desc;
	ALuint m_format;
	ALuint m_buffers[4];
	ALuint m_source;
	uint32_t m_submitted;
	AutoArrayPtr< uint8_t > m_data;
};

}

