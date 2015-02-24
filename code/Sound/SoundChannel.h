#ifndef traktor_sound_SoundChannel_H
#define traktor_sound_SoundChannel_H

#include "Core/Object.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Thread/DoubleBuffer.h"
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

	/*! \brief Set channel volume. */
	void setVolume(float volume);

	/*! \brief Get channel volume. */
	float getVolume() const;

	/*! \brief Set pitch. */
	void setPitch(float pitch);

	/*! \brief Get current pitch. */
	float getPitch() const;

	/*! \brief Associate filter in channel. */
	void setFilter(const IFilter* filter);

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
	 * \param repeat Number of times to repeat sound.
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
	ISoundBufferCursor* getCursor();

private:
	friend class SoundSystem;

	struct StateFilter
	{
		Ref< const IFilter > filter;
		Ref< IFilterInstance > filterInstance;
	};

	struct StateSound
	{
		Ref< const ISoundBuffer > buffer;
		Ref< ISoundBufferCursor > cursor;
		handle_t category;
		float volume;
		float presence;
		float presenceRate;
		uint32_t repeat;

		StateSound()
		:	category(0)
		,	volume(1.0f)
		,	presence(0.0f)
		,	presenceRate(1.0f)
		,	repeat(0)
		{
		}
	};

	struct StateParameter
	{
		CircularVector< std::pair< handle_t, float >, 4 > set;
	};

	uint32_t m_id;
	uint32_t m_hwSampleRate;	//< Hardware sample rate.
	uint32_t m_hwFrameSamples;	//< Hardware frame size in samples.
	float m_volume;
	float m_pitch;
	bool m_playing;
	bool m_allowRepeat;
	DoubleBuffer< StateFilter > m_stateFilter;
	DoubleBuffer< StateSound > m_stateSound;
	DoubleBuffer< StateParameter > m_stateParameters;
	float* m_outputSamples[SbcMaxChannelCount];
	uint32_t m_outputSamplesIn;

	SoundChannel(uint32_t id, uint32_t hwSampleRate, uint32_t hwFrameSamples);

	bool getBlock(
		const ISoundMixer* mixer,
		double time,
		SoundBlock& outBlock,
		SoundBlockMeta& outBlockMeta
	);
};

	}
}

#endif	// traktor_sound_SoundChannel_H
