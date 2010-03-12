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
 */
class T_DLLCLASS ISoundMixer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void mulConst(float* sb, uint32_t count, float factor) const = 0;

	virtual void mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const = 0;

	virtual void addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const = 0;

	virtual void mute(float* sb, uint32_t count) const = 0;
};

	}
}

#endif	// traktor_sound_ISoundMixer_H
