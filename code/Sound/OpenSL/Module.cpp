#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/OpenSL/AudioDriverOpenSL.h"

namespace traktor
{
	namespace sound
	{

extern "C" void __module__Traktor_Sound_OpenSL_ES()
{
	T_FORCE_LINK_REF(AudioDriverOpenSL);
}

	}
}

#endif
