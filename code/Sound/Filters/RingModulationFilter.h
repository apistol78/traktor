#ifndef traktor_sound_RingModulationFilter_H
#define traktor_sound_RingModulationFilter_H

#include "Sound/IFilter.h"

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

/*! \brief Ring modulation filter.
 * \ingroup Sound
 */
class T_DLLCLASS RingModulationFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	RingModulationFilter(uint32_t sampleRate = 44100, uint32_t ringFrequency = 2500);

	virtual Ref< IFilterInstance > createInstance() const;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const;

	virtual void serialize(ISerializer& s);

private:
	uint32_t m_sampleRate;
	uint32_t m_ringFrequency;
};

	}
}

#endif	// traktor_sound_RingModulationFilter_H
