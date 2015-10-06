#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Input/Di8/InputDriverDi8.h"

namespace traktor
{
	namespace input
	{

extern "C" void __module__Traktor_Input_Di8()
{
	T_FORCE_LINK_REF(InputDriverDi8);
}

	}
}

#endif
