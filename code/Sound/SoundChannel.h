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
	virtual ~SoundChannel();

	/*! \brief Set channel volume. */
	void setVolume(float volume);

	/*! \brief Associate filter in channel. */
	void setFilter(IFilter* filter);

	/*! \brief Get currently associated filter. */
	IFilter* getFilter() const;

	/*! \brief Set pitch. */
	void setPitch(float pitch);

	/*! \brief Get current pitch. */
	float getPitch() const;

	/*! \brief Set cursor parameter. */
	void setParameter(float parameter);

	/*! \brief Play sound through this channel. */
	bool play(const Sound* sound, uint32_t repeat = 0);

	/*! \brief Check if there are a sound playing in this channel. */
	bool isPlaying() const;

	/*! \brief Stop playing sound. */
	void stop();

	/*! \brief Return current playing sound's cursor. */
	ISoundBufferCursor* getCursor() const;

private:
	friend class SoundSystem;

	struct State
	{
		Ref< const Sound > sound;
		Ref< ISoundBufferCursor > cursor;
		Ref< IFilter > filter;
		Ref< IFilterInstance > filterInstance;
		uint32_t repeat;
		float presence;

		State()
		:	repeat(0)
		,	presence(0.0f)
		{
		}
	};

	uint32_t m_id;
	Event& m_eventFinish;
	uint32_t m_hwSampleRate;	//< Hardware sample rate.
	uint32_t m_hwFrameSamples;	//< Hardware frame size in samples.
	Semaphore m_lock;
	State m_currentState;
	State m_activeState;
	float* m_outputSamples[SbcMaxChannelCount];
	uint32_t m_outputSamplesIn;
	float m_pitch;
	float m_volume;

	SoundChannel(uint32_t id, Event& eventFinish, uint32_t hwSampleRate, uint32_t hwFrameSamples);

	bool getBlock(const ISoundMixer* mixer, double time, SoundBlock& outBlock);
};

	}
}

#endif	// traktor_sound_SoundChannel_H
