#ifndef traktor_sound_SoundMixer_H
#define traktor_sound_SoundMixer_H

#include "Sound/ISoundMixer.h"

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

/*! \brief Default sound mixer implementation.
 * \ingroup Sound
 */
class T_DLLCLASS SoundMixer : public ISoundMixer
{
	T_RTTI_CLASS;

public:
	virtual void mulConst(float* sb, uint32_t count, float factor) const;

	virtual void mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const;

	virtual void addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const;

	virtual void stretch(float* lsb, uint32_t lcount, const float* rsb, uint32_t rcount, float factor) const;

	virtual void mute(float* sb, uint32_t count) const;

	virtual void synchronize() const;
};

	}
}

#endif	// traktor_sound_SoundMixer_H
