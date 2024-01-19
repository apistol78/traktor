/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/Player/ISoundHandle.h"

namespace traktor::sound
{

class SoundHandle : public ISoundHandle
{
	T_RTTI_CLASS;

public:
	virtual void stop() override final;

	virtual void fadeOff() override final;

	virtual bool isPlaying() override final;

	virtual void setVolume(float volume) override final;

	virtual void setPitch(float pitch) override final;

	virtual void setPosition(const Vector4& position) override final;

	virtual void setParameter(int32_t id, float parameter) override final;

	virtual IAudioBufferCursor* getCursor() override final;

private:
	friend class SoundPlayer;

	AudioChannel* m_channel;
	Vector4* m_position;
	float* m_fadeOff;

	SoundHandle(AudioChannel* channel, Vector4& position, float& fadeOff);

	void detach();
};

}
