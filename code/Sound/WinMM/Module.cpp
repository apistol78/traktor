#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/WinMM/AudioDriverWinMM.h"

namespace traktor
{
	namespace sound
	{

extern "C" void __module__Traktor_Sound_WinMM()
{
	T_FORCE_LINK_REF(AudioDriverWinMM);
}

	}
}

#endif
