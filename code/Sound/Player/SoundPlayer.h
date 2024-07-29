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
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Timer/Timer.h"

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
class Sound;
class SoundHandle;
class SoundListener;
class SurroundEnvironment;
class SurroundFilter;

/*! High-level sound player implementation.
 * \ingroup Sound
 */
class T_DLLCLASS SoundPlayer : public Object
{
	T_RTTI_CLASS;

public:
	SoundPlayer();

	bool create(AudioSystem* audioSystem, SurroundEnvironment* surroundEnvironment);

	void destroy();

	Ref< SoundHandle > play(const Sound* sound, uint32_t priority);

	Ref< SoundHandle > play(const Sound* sound, const Vector4& position, uint32_t priority, bool autoStopFar);

	void addListener(const SoundListener* listener);

	void removeListener(const SoundListener* listener);

	void update(float dT);

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

