#ifndef traktor_sound_SoundBlockUtilities_H
#define traktor_sound_SoundBlockUtilities_H

#include "Sound/Types.h"

namespace traktor
{
	namespace sound
	{

void soundBlockMulConst(SoundBlock& sb, float factor);

void soundBlockMulConst(float* lsb, const float* rsb, uint32_t count, float factor);

void soundBlockAddMulConst(float* lsb, const float* rsb, uint32_t count, float factor);

void soundBlockMute(float* sb, uint32_t count);

	}
}

#endif	// traktor_sound_SoundBlockUtilities_H
