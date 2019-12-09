#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Timer/Timer.h"
#include "Sound/Player/ISoundPlayer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class AudioChannel;
class LowPassFilter;
class SoundHandle;
class SoundSystem;
class SurroundEnvironment;
class SurroundFilter;

/*! \brief High-level sound player implementation.
 * \ingroup Sound
 */
class T_DLLCLASS SoundPlayer : public ISoundPlayer
{
	T_RTTI_CLASS;

public:
	SoundPlayer();

	bool create(SoundSystem* soundSystem, SurroundEnvironment* surroundEnvironment);

	virtual void destroy() override final;

	virtual Ref< ISoundHandle > play(const Sound* sound, uint32_t priority) override final;

	virtual Ref< ISoundHandle > play(const Sound* sound, const Vector4& position, uint32_t priority, bool autoStopFar) override final;

	virtual void setListenerTransform(const Transform& listenerTransform) override final;

	virtual Transform getListenerTransform() const override final;

	virtual void update(float dT) override final;

private:
	struct Channel
	{
		Vector4 position;
		Ref< SurroundFilter > surroundFilter;
		Ref< LowPassFilter > lowPassFilter;
		Ref< const Sound > sound;
		AudioChannel* audioChannel;
		uint32_t priority;
		float fadeOff;
		float time;
		bool autoStopFar;
		Ref< SoundHandle > handle;
	};

	mutable Semaphore m_lock;
	Ref< SoundSystem > m_soundSystem;
	Ref< SurroundEnvironment > m_surroundEnvironment;
	AlignedVector< Channel > m_channels;
	Timer m_timer;
};

	}
}

