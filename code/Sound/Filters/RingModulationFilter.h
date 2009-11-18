#ifndef traktor_sound_RingModulationFilter_H
#define traktor_sound_RingModulationFilter_H

#include "Sound/IFilter.h"

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

/*! \brief Ring modulation filter.
 * \ingroup Sound
 */
class T_DLLCLASS RingModulationFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	RingModulationFilter(uint32_t sampleRate, uint32_t ringFrequency = 2500);

	virtual void apply(SoundBlock& outBlock);

private:
	uint32_t m_sampleRate;
	uint32_t m_ringFrequency;
	float m_time;
};

	}
}

#endif	// traktor_sound_RingModulationFilter_H
