/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class ISoundHandle;
class ISoundListener;
class Sound;

/*! High-level sound player.
 * \ingroup Sound
 */
class T_DLLCLASS ISoundPlayer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	/*! Play global sound.
	 */
	virtual Ref< ISoundHandle > play(const Sound* sound, uint32_t priority) = 0;

	/*! Play positional sound.
	 */
	virtual Ref< ISoundHandle > play(const Sound* sound, const Vector4& position, uint32_t priority, bool autoStopFar) = 0;

	/*! */
	virtual Ref< ISoundListener > createListener() const = 0;

	/*! */
	virtual void addListener(const ISoundListener* listener) = 0;

	/*! */
	virtual void removeListener(const ISoundListener* listener) = 0;

	/*! Update sound player.
	 */
	virtual void update(float dT) = 0;
};

}
