#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/Ds8/SoundDriverDs8.h"

namespace traktor
{
	namespace sound
	{

extern "C" void __module__Traktor_Sound_Ds8()
{
	T_FORCE_LINK_REF(SoundDriverDs8);
}

	}
}

#endif
