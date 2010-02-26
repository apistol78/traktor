#ifndef traktor_sound_SoundChannel_H
#define traktor_sound_SoundChannel_H

#include "Core/Object.h"
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
	SoundChannel(uint32_t hwSampleRate, uint32_t hwFrameSamples);

	virtual ~SoundChannel();

	void setVolume(float volume);

	void setFilter(IFilter* filter);

	Ref< IFilter > getFilter() const;

	bool isPlaying() const;

	void stop();

	ISoundBufferCursor* getCursor() const;

private:
	friend class SoundSystem;

	uint32_t m_hwSampleRate;	//< Hardware sample rate.
	uint32_t m_hwFrameSamples;	//< Hardware frame size in samples.
	Semaphore m_filterLock;
	Ref< IFilter > m_filter;
	Ref< IFilterInstance > m_filterInstance;
	Ref< Sound > m_sound;
	Ref< ISoundBufferCursor > m_cursor;
	double m_time;
	uint32_t m_repeat;
	
	float* m_outputSamples[SbcMaxChannelCount];
	uint32_t m_outputSamplesIn;

	float m_volume;

	void playSound(Sound* sound, double time, uint32_t repeat);

	bool getBlock(double time, SoundBlock& outBlock);
};

	}
}

#endif	// traktor_sound_SoundChannel_H
