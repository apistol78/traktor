/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Sound/IAudioDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::sound
{

/*! Write-out device audio driver.
 * \ingroup Sound
 */
class T_DLLCLASS AudioDriverWriteOut : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	AudioDriverWriteOut(IAudioDriver* childDriver = nullptr);

	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const SoundBlock& soundBlock) override final;

private:
	Ref< IAudioDriver > m_childDriver;
	AudioDriverCreateDesc m_desc;
	Ref< IStream > m_stream;
	AutoArrayPtr< float > m_interleaved;
	bool m_wait;
};

}
