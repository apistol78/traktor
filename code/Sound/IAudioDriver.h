/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class IAudioMixer;

/*! Audio driver.
 * \ingroup Sound
 *
 * Audio driver works as follows:
 *  -# Audio system submits a sound block.
 *  -# Audio system waits until driver is ready for another block.
 *  -# Repeat from step 1.
 *
 * Note that even though there are no audio actually playing the
 * audio system keeps on passing "muted" sound blocks.
 *
 * A sound block passed to the driver is always of a fixed size
 * determined by the "frameSamples" member in AudioDriverCreateDesc structure.
 *
 * It the drivers responsibility to ensure that there are no glitches in the
 * playback by any means necessary.
 */
class T_DLLCLASS IAudioDriver : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create sound driver.
	 *
	 * \param desc Sound driver create description.
	 * \param outMixer Return alternative mixer implementation.
	 * \return True if successful.
	 */
	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) = 0;

	virtual void destroy() = 0;

	virtual void wait() = 0;

	virtual void submit(const SoundBlock& soundBlock) = 0;
};

}
