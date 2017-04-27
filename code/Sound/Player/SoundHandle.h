/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SoundHandle_H
#define traktor_sound_SoundHandle_H

#include "Sound/Player/ISoundHandle.h"

namespace traktor
{
	namespace sound
	{

class SoundHandle : public ISoundHandle
{
	T_RTTI_CLASS;

public:
	virtual void stop() T_OVERRIDE T_FINAL;

	virtual void fadeOff() T_OVERRIDE T_FINAL;

	virtual bool isPlaying() T_OVERRIDE T_FINAL;

	virtual void setVolume(float volume) T_OVERRIDE T_FINAL;

	virtual void setPitch(float pitch) T_OVERRIDE T_FINAL;

	virtual void setPosition(const Vector4& position) T_OVERRIDE T_FINAL;

	virtual void setParameter(int32_t id, float parameter) T_OVERRIDE T_FINAL;

private:
	friend class SoundPlayer;

	SoundChannel* m_channel;
	Vector4* m_position;
	float* m_fadeOff;

	SoundHandle(SoundChannel* channel, Vector4& position, float& fadeOff);

	void detach();
};

	}
}

#endif	// traktor_sound_SoundHandle_H
