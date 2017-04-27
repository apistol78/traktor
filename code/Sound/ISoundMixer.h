/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_ISoundMixer_H
#define traktor_sound_ISoundMixer_H

#include "Core/Object.h"

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

/*! \brief Sound mixer interface.
 * \ingroup Sound
 *
 * Following restrictions for use with this interface apply:
 * - Sample blocks must be 16-byte aligned.
 * - Counts must be a multiple of 4.
 */
class T_DLLCLASS ISoundMixer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void mulConst(float* sb, uint32_t count, float factor) const = 0;

	virtual void mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const = 0;

	virtual void addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const = 0;

	virtual void stretch(float* lsb, uint32_t lcount, const float* rsb, uint32_t rcount, float factor) const = 0;

	virtual void mute(float* sb, uint32_t count) const = 0;

	virtual void synchronize() const = 0;
};

	}
}

#endif	// traktor_sound_ISoundMixer_H
