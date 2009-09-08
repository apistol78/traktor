#ifndef traktor_sound_CombFilter_H
#define traktor_sound_CombFilter_H

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

/*! \brief Comb filter.
 * \ingroup Sound
 */
class T_DLLCLASS CombFilter : public IFilter
{
	T_RTTI_CLASS(CombFilter)

public:
	CombFilter(uint32_t samplesLength, float feedback, float damp);

	virtual void apply(SoundBlock& outBlock);

private:
	float m_feedback;
	float m_damp;
	std::vector< float > m_history[2];
	float m_last[2];
	uint32_t m_index[2];
};

	}
}

#endif	// traktor_sound_CombFilter_H
