#ifndef traktor_sound_EqualizerFilter_H
#define traktor_sound_EqualizerFilter_H

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

/*! \brief Equalizer filter.
 * \ingroup Sound
 */
class T_DLLCLASS EqualizerFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	EqualizerFilter(float gain = 0.0f);

	virtual Ref< IFilterInstance > createInstance() const;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const;

	virtual void serialize(ISerializer& s);

private:
	float m_gain;
};

	}
}

#endif	// traktor_sound_EqualizerFilter_H
