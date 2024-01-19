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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Vector4;

}

namespace traktor::sound
{

class IAudioBufferCursor;

class T_DLLCLASS ISoundHandle : public Object
{
	T_RTTI_CLASS;

public:
	virtual void stop() = 0;

	virtual void fadeOff() = 0;

	virtual bool isPlaying() = 0;

	virtual void setVolume(float volume) = 0;

	virtual void setPitch(float pitch) = 0;

	virtual void setPosition(const Vector4& position) = 0;

	virtual void setParameter(int32_t id, float parameter) = 0;

	virtual IAudioBufferCursor* getCursor() = 0;
};

}
