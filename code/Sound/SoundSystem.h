#ifndef traktor_sound_SoundSystem_H
#define traktor_sound_SoundSystem_H

#include <list>
#include "Core/Object.h"
#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Event.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class ISoundDriver;
class SoundChannel;
class Sound;

/*! \brief Sound system manager.
 * \ingroup Sound
 *
 * The SoundSystem class manages mixing sound blocks
 * from virtual channels and feeding them through the
 * submission thread into the sound driver for playback.
 */
class T_DLLCLASS SoundSystem : public Object
{
	T_RTTI_CLASS(SoundSystem)

public:
	SoundSystem(ISoundDriver* driver);

	bool create(const SoundSystemCreateDesc& desc);

	void destroy();

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

	/*! \brief Play sound, override playing sound if channel is already playing.
	 *
	 * \param sound Sound to play.
	 * \param repeat Repeat sound.
	 * \return Sound channel.
	 */
	Ref< SoundChannel > play(uint32_t channelId, Sound* sound, uint32_t repeat = 1);

	/*! \brief Play sound on first non-playing virtual channel.
	 *
	 * \param sound Sound to play.
	 * \param wait Wait for virtual channel to become available if all are currently playing.
	 * \param repeat Repeat sound.
	 * \return Sound channel if sound was successfully attached to one.
	 */
	Ref< SoundChannel > play(Sound* sound, bool wait, uint32_t repeat = 1);

	/*! \brief Stop virtual channel from playing.
	 *
	 * \param channelId Virtual channel identifier.
	 */
	void stop(uint32_t channelId);

	/*! \brief Stop playing all virtual channels. */
	void stopAll();

	/*! \brief Get current mixer time. */
	double getTime() const;

	/*! \brief Query performance of each thread. */
	void getThreadPerformances(double& outMixerTime, double& outSubmitTime) const;

private:
	Ref< ISoundDriver > m_driver;
	SoundSystemCreateDesc m_desc;
	Thread* m_threadMixer;
	Thread* m_threadSubmit;
	Semaphore m_channelAttachLock;
	RefArray< SoundChannel > m_channels;
	Semaphore m_submitQueueLock;
	Event m_submitQueueEvent;
	Event m_submitConsumedEvent;
	std::list< SoundBlock > m_submitQueue;
	Semaphore m_samplesBlocksLock;
	std::vector< float* > m_samplesBlocks;
	double m_time;
	double m_mixerThreadTime;
	double m_submitThreadTime;

	void threadMixer();

	void threadSubmit();
};

	}
}

#endif	// traktor_sound_SoundSystem_H
