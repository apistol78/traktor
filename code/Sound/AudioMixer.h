#pragma once

#include "Sound/IAudioMixer.h"

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

/*! Default sound mixer implementation.
 * \ingroup Sound
 */
class T_DLLCLASS AudioMixer : public IAudioMixer
{
	T_RTTI_CLASS;

public:
	virtual void mulConst(float* sb, uint32_t count, float factor) const override final;

	virtual void mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const override final;

	virtual void addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const override final;

	virtual void stretch(float* lsb, uint32_t lcount, const float* rsb, uint32_t rcount, float factor) const override final;

	virtual void mute(float* sb, uint32_t count) const override final;

	virtual void synchronize() const override final;
};

	}
}

