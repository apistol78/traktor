#pragma once

#include "Sound/Player/ISoundHandle.h"

namespace traktor
{
	namespace sound
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

