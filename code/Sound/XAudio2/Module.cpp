#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/XAudio2/AudioDriverXAudio2.h"

namespace traktor
{
	namespace sound
	{

extern "C" void __module__Traktor_Sound_XAudio2()
{
	T_FORCE_LINK_REF(AudioDriverXAudio2);
}

	}
}

#endif
