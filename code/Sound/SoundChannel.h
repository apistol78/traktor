#ifndef traktor_sound_SoundChannel_H
#define traktor_sound_SoundChannel_H

#include "Core/Object.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Semaphore.h"
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

struct IFilterInstance;

class IFilter;
class ISoundBufferCursor;
class ISoundMixer;
class Sound;

/*! \brief Virtual sound channel.
 * \ingroup Sound
 *
 * The sound system manages a given number of virtual
 * sound channels which is responsible of managing
 * sounds attached to them and also calling appropriate
 * filters.
 */
class T_DLLCLASS SoundChannel : public Object
{
	T_RTTI_CLASS;

public:
	SoundChannel(uint32_t id, Event& eventFinish, uint32_t hwSampleRate, uint32_t hwFrameSamples);

	virtual ~SoundChannel();

	/*! \brief Set channel volume. */
	void setVolume(float volume);

	/*! \brief Associate filter in channel. */
	void setFilter(IFilter* filter);

	/*! \brief Get currently associated filter. */
	IFilter* getFilter() const;

	/*! \brief Set exclusive flag.
	 *
	 * When channel is marked as exclusive then
	 * sound system won't acquire this channel
	 * for sounds even if the channel isn't playing any sounds.
	 *
	 * \param exclusive True if channel is application exclusive.
	 */
	void setExclusive(bool exclusive);

	/*! \brief Check if channel is application exclusive. */
	bool isExclusive() const;

	/*! \brief Check if there are a sound playing in this channel. */
	bool isPlaying() const;

	/*! \brief Stop playing sound. */
	void stop();

	/*! \brief Return current playing sound's cursor. */
	ISoundBufferCursor* getCursor() const;

private:
	friend class SoundSystem;

	uint32_t m_id;
	Event& m_eventFinish;
	uint32_t m_hwSampleRate;	//< Hardware sample rate.
	uint32_t m_hwFrameSamples;	//< Hardware frame size in samples.
	Semaphore m_lock;
	Ref< IFilter > m_filter;
	Ref< IFilterInstance > m_filterInstance;
	Ref< const Sound > m_sound;
	Ref< ISoundBufferCursor > m_cursor;
	uint32_t m_priority;
	uint32_t m_repeat;
	float* m_outputSamples[SbcMaxChannelCount];
	uint32_t m_outputSamplesIn;
	float m_volume;
	bool m_exclusive;

	bool playSound(const Sound* sound, double time, uint32_t priority, uint32_t repeat);

	bool getBlock(const ISoundMixer* mixer, double time, SoundBlock& outBlock);

	uint32_t getPriority() const;
};

	}
}

#endif	// traktor_sound_SoundChannel_H
