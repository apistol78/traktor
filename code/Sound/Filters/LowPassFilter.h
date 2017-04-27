/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_LowPassFilter_H
#define traktor_sound_LowPassFilter_H

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

/*! \brief Low pass filter.
 * \ingroup Sound
 */
class T_DLLCLASS LowPassFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	LowPassFilter(float cutOff = 1e+8f);

	virtual Ref< IFilterInstance > createInstance() const T_OVERRIDE T_FINAL;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	inline void setCutOff(float cutOff) { m_cutOff = cutOff; }

	inline float getCutOff() const { return m_cutOff; }

private:
	float m_cutOff;
};

	}
}

#endif	// traktor_sound_LowPassFilter_H
