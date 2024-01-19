/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

namespace traktor::sound
{

class AudioChannel;
class AudioSystem;
class LowPassFilter;
class SoundHandle;
class SoundListener;
class SurroundEnvironment;
class SurroundFilter;

/*! High-level sound player implementation.
 * \ingroup Sound
 */
class T_DLLCLASS SoundPlayer : public ISoundPlayer
{
	T_RTTI_CLASS;

public:
	SoundPlayer();

	bool create(AudioSystem* audioSystem, SurroundEnvironment* surroundEnvironment);

	virtual void destroy() override final;

	virtual Ref< ISoundHandle > play(const Sound* sound, uint32_t priority) override final;

	virtual Ref< ISoundHandle > play(const Sound* sound, const Vector4& position, uint32_t priority, bool autoStopFar) override final;

	virtual Ref< ISoundListener > createListener() const override final;

	virtual void addListener(const ISoundListener* listener) override final;

	virtual void removeListener(const ISoundListener* listener) override final;

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
	Ref< AudioSystem > m_audioSystem;
	Ref< SurroundEnvironment > m_surroundEnvironment;
	RefArray< const SoundListener > m_listeners;
	AlignedVector< Channel > m_channels;
	Timer m_timer;
};

}

