/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_DitherFilter_H
#define traktor_sound_DitherFilter_H

#include "Sound/IFilter.h"
#include "Core/Math/Random.h"

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

/*! \brief Dither filter.
 * \ingroup Sound
 */
class T_DLLCLASS DitherFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	DitherFilter(uint32_t bitsPerSample = 16);

	virtual Ref< IFilterInstance > createInstance() const T_OVERRIDE T_FINAL;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	float m_ditherAmplitude;
	mutable Random m_random;
};

	}
}

#endif	// traktor_sound_DitherFilter_H
