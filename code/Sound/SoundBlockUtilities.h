#ifndef traktor_sound_SoundBlockUtilities_H
#define traktor_sound_SoundBlockUtilities_H

#include "Sound/Types.h"

namespace traktor
{
	namespace sound
	{

void soundBlockMulConst(SoundBlock& sb, float factor);

void soundBlockAddMulConst(SoundBlock& lsb, int lch, const SoundBlock& rsb, int32_t rch, float factor);

	}
}

#endif	// traktor_sound_SoundBlockUtilities_H
