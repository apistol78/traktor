#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/Ps3/SoundDriverPs3.h"

namespace traktor
{
	namespace sound
	{

extern "C" void __module__Traktor_Sound_Ps3()
{
	T_FORCE_LINK_REF(SoundDriverPs3);
}

	}
}

#endif
