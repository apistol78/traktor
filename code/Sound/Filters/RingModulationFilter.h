/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual Ref< IFilterInstance > createInstance() const T_OVERRIDE T_FINAL;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint32_t m_sampleRate;
	uint32_t m_ringFrequency;
};

	}
}

#endif	// traktor_sound_RingModulationFilter_H
