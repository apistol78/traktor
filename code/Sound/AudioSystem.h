/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Thread.h"
#include "Sound/Types.h"

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
class IAudioDriver;
class IAudioMixer;
class Sound;

/*! Audio system manager.
 * \ingroup Sound
 *
 * The AudioSystem class manages mixing sounds
 * from virtual channels and feeding them through the
 * submission thread into the audio driver for playback.
 */
class T_DLLCLASS AudioSystem : public Object
{
	T_RTTI_CLASS;

public:
	explicit AudioSystem(IAudioDriver* driver);

	/*! Create audio system.
	 *
	 * \param desc Initialize description.
	 * \return True if audio system created successfully.
	 */
	bool create(const SoundSystemCreateDesc& desc);

	/*! Destroy audio system.
	 */
	void destroy();

	/*! Reset audio system.
	 *
	 * \param driver New audio driver implementation.
	 * \return True if reset succeeded.
	 */
	bool reset(IAudioDriver* driver);

	/*! Suspend playback.
	 */
	void suspend();

	/*! Resume playback.
	 */
	void resume();

	/*! Set global volume.
	 *
	 * \param volume Volume (0-1).
	 */
	void setVolume(float volume);

	/*! Get global volume.
	 *
	 * \return Global volume (0-1).
	 */
	float getVolume() const;

	/*! Set category volume.
	 *
	 * \param volume Volume (0-1).
	 */
	void setVolume(handle_t category, float volume);

	/*! Get category volume.
	 *
	 * \return Category volume.
	 */
	float getVolume(handle_t category) const;

	/*! Set global combination matrix.
	 *
	 * [hardware channel][virtual channel]
	 *
	 * \param cm Combination matrix.
	 */
	void setCombineMatrix(float cm[SbcMaxChannelCount][SbcMaxChannelCount]);

	/*! Get virtual channel.
	 *
	 * \param channelId Virtual channel identifier.
	 * \return Virtual sound channel.
	 */
	AudioChannel* getChannel(uint32_t channelId);

	/*! Get current mixer time.
	 *
	 * \return Mixer time in seconds.
	 */
	double getTime() const;

	/*! Query performance of each thread.
	 *
	 * \param outMixerTime Last mixer thread duration in seconds.
	 */
	void getThreadPerformances(double& outMixerTime) const;

private:
	Ref< IAudioDriver > m_driver;
	Ref< IAudioMixer > m_mixer;
	SoundSystemCreateDesc m_desc;
	bool m_suspended;
	float m_volume;
	SmallMap< handle_t, float > m_categoryVolumes;
	Thread* m_threadMixer;
	RefArray< AudioChannel > m_channels;
	AlignedVector< SoundBlock > m_requestBlocks;

	// \name Submission queue
	// \{

	Semaphore m_channelsLock;

	// \}

	// \name Mixer data blocks
	// \{

	float* m_samplesData;
	CircularVector< float*, 4 > m_samplesBlocks;

	// \}

	double m_time;
	double m_mixerThreadTime;

	void threadMixer();
};

	}
}

