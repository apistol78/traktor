/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_ReverbFilter_H
#define traktor_sound_ReverbFilter_H

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

/*! \brief Reverb filter.
 * \ingroup Sound
 */
class T_DLLCLASS ReverbFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	ReverbFilter();

	ReverbFilter(
		int32_t delay,
		float duration,
		float cutOff,
		float wet
	);

	virtual Ref< IFilterInstance > createInstance() const T_OVERRIDE T_FINAL;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	int32_t m_delay;
	float m_duration;
	float m_cutOff;
	float m_wet;
};

	}
}

#endif	// traktor_sound_ReverbFilter_H
