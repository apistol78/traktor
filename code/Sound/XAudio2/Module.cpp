#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/XAudio2/SoundDriverXAudio2.h"

namespace traktor
{
	namespace sound
	{

extern "C" void __module__Traktor_Sound_XAudio2()
{
	T_FORCE_LINK_REF(SoundDriverXAudio2);
}

	}
}

#endif
