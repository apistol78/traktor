#ifndef traktor_sound_SoundChannel_H
#define traktor_sound_SoundChannel_H

#include "Core/Object.h"
#include "Core/Containers/CircularVector.h"
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
class ISoundBuffer;
class ISoundBufferCursor;
class ISoundMixer;

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

	/*! \brief Get category. */
	handle_t getCategory() const;

	/*! \brief Set channel volume. */
	void setVolume(float volume);

	/*! \brief Associate filter in channel. */
	void setFilter(const IFilter* filter);

	/*! \brief Get currently associated filter. */
	const IFilter* getFilter() const;

	/*! \brief Set pitch. */
	void setPitch(float pitch);

	/*! \brief Get current pitch. */
	float getPitch() const;

	/*! \brief Set cursor parameter. */
	void setParameter(handle_t id, float parameter);

	/*! \brief Disable repeat on current sound. */
	void disableRepeat();

	/*! \brief Play sound through this channel.
	 *
	 * \param buffer Sound buffer.
	 * \param category Sound category.
	 * \param volume Sound volume.
	 * \param presence Sound presence.
	 * \param presenceRate Sound presence recover rate.
	 * \param repeat Number of times to repreat sound.
	 * \return True if sound is playing successfully.
	 */
	bool play(
		const ISoundBuffer* buffer,
		handle_t category,
		float volume,
		float presence,
		float presenceRate,
		uint32_t repeat = 0
	);

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
		Ref< const ISoundBuffer > buffer;
		Ref< ISoundBufferCursor > cursor;
		Ref< const IFilter > filter;
		Ref< IFilterInstance > filterInstance;
		handle_t category;
		float volume;
		float pitch;
		float presence;
		float presenceRate;
		uint32_t repeat;

		State()
		:	category(0)
		,	volume(1.0f)
		,	pitch(1.0f)
		,	presence(0.0f)
		,	presenceRate(1.0f)
		,	repeat(0)
		{
		}
	};

	struct ParameterQueue
	{
		handle_t id;
		float parameter;

		ParameterQueue()
		{
		}

		ParameterQueue(handle_t id_, float parameter_)
		:	id(id_)
		,	parameter(parameter_)
		{
		}
	};

	uint32_t m_id;
	Event& m_eventFinish;
	uint32_t m_hwSampleRate;	//< Hardware sample rate.
	uint32_t m_hwFrameSamples;	//< Hardware frame size in samples.
	Semaphore m_lock;
	Semaphore m_parameterQueueLock;
	CircularVector< ParameterQueue, 4 > m_parameterQueue;
	float m_volume;
	State m_state;
	float* m_outputSamples[SbcMaxChannelCount];
	uint32_t m_outputSamplesIn;

	SoundChannel(uint32_t id, Event& eventFinish, uint32_t hwSampleRate, uint32_t hwFrameSamples);

	bool getBlock(const ISoundMixer* mixer, double time, SoundBlock& outBlock);
};

	}
}

#endif	// traktor_sound_SoundChannel_H
