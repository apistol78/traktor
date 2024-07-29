/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class AudioChannel;
class IAudioBufferCursor;

class T_DLLCLASS SoundHandle : public Object
{
	T_RTTI_CLASS;

public:
	void stop();

	void fadeOff();

	bool isPlaying();

	void setVolume(float volume);

	void setPitch(float pitch);

	void setPosition(const Vector4& position);

	void setParameter(int32_t id, float parameter);

	IAudioBufferCursor* getCursor();

private:
	friend class SoundPlayer;

	AudioChannel* m_channel;
	Vector4* m_position;
	float* m_fadeOff;

	SoundHandle(AudioChannel* channel, Vector4& position, float& fadeOff);

	void detach();
};

}
