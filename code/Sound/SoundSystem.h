/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SoundSystem_H
#define traktor_sound_SoundSystem_H

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

class ISoundDriver;
class ISoundMixer;
class Sound;
class SoundChannel;

/*! \brief Sound system manager.
 * \ingroup Sound
 *
 * The SoundSystem class manages mixing sound blocks
 * from virtual channels and feeding them through the
 * submission thread into the sound driver for playback.
 */
class T_DLLCLASS SoundSystem : public Object
{
	T_RTTI_CLASS;

public:
	SoundSystem(ISoundDriver* driver);

	/*! \brief Create sound system.
	 *
	 * \param desc Initialize description.
	 * \return True if sound system created successfully.
	 */
	bool create(const SoundSystemCreateDesc& desc);

	/*! \brief Destroy sound system.
	 */
	void destroy();

	/*! \brief Reset sound system.
	 *
	 * \param driver New sound driver implementation.
	 * \return True if reset succeeded.
	 */
	bool reset(ISoundDriver* driver);

	/*! \brief Suspend playback.
	 */
	void suspend();

	/*! \brief Resume playback.
	 */
	void resume();

	/*! \brief Set global volume.
	 *
	 * \param volume Volume (0-1).
	 */
	void setVolume(float volume);

	/*! \brief Get global volume.
	 *
	 * \return Global volume (0-1).
	 */
	float getVolume() const;

	/*! \brief Set category volume.
	 *
	 * \param volume Volume (0-1).
	 */
	void setVolume(handle_t category, float volume);

	/*! \brief Get category volume.
	 *
	 * \return Category volume.
	 */
	float getVolume(handle_t category) const;

	/*! \brief Set global combination matrix.
	 *
	 * [hardware channel][virtual channel]
	 *
	 * \param cm Combination matrix.
	 */
	void setCombineMatrix(float cm[SbcMaxChannelCount][SbcMaxChannelCount]);

	/*! \brief Get virtual channel.
	 *
	 * \param channelId Virtual channel identifier.
	 * \return Virtual sound channel.
	 */
	Ref< SoundChannel > getChannel(uint32_t channelId);

	/*! \brief Get current mixer time.
	 *
	 * \return Mixer time in seconds.
	 */
	double getTime() const;

	/*! \brief Query performance of each thread.
	 *
	 * \param outMixerTime Last mixer thread duration in seconds.
	 */
	void getThreadPerformances(double& outMixerTime) const;

private:
	Ref< ISoundDriver > m_driver;
	Ref< ISoundMixer > m_mixer;
	SoundSystemCreateDesc m_desc;
	bool m_suspended;
	float m_volume;
	SmallMap< handle_t, float > m_categoryVolumes;
	Thread* m_threadMixer;
	RefArray< SoundChannel > m_channels;
	AlignedVector< SoundBlock > m_requestBlocks;
	AlignedVector< SoundBlockMeta > m_requestBlockMeta;

	// \name Submission queue
	// \{

	Semaphore m_channelsLock;

	// \}
	
	// \name Mixer data blocks
	// \{

	float* m_samplesData;
	CircularVector< float*, 4 > m_samplesBlocks;
	AlignedVector< float > m_duck[2];

	// \}

	double m_time;
	double m_mixerThreadTime;

	void threadMixer();
};

	}
}

#endif	// traktor_sound_SoundSystem_H
