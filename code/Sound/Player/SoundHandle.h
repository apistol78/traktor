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
	virtual void stop();

	virtual bool isPlaying();

	virtual void setVolume(float volume);

	virtual void setPitch(float pitch);

	virtual void setPosition(const Vector4& position);

	virtual void setParameter(int32_t id, float parameter);

private:
	friend class SoundPlayer;

	SoundChannel* m_channel;
	Vector4* m_position;

	SoundHandle(SoundChannel* channel, Vector4& position);

	void detach();
};

	}
}

#endif	// traktor_sound_SoundHandle_H
