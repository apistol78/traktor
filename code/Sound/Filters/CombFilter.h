#ifndef traktor_sound_CombFilter_H
#define traktor_sound_CombFilter_H

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

/*! \brief Comb filter.
 * \ingroup Sound
 */
class T_DLLCLASS CombFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	CombFilter(uint32_t samplesLength = 10, float feedback = 0.0f, float damp = 0.0f);

	virtual Ref< IFilterInstance > createInstance() const;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const;

	virtual void serialize(ISerializer& s);

private:
	uint32_t m_samplesLength;
	float m_feedback;
	float m_damp;
};

	}
}

#endif	// traktor_sound_CombFilter_H
