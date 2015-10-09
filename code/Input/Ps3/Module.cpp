#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Input/Ps3/InputDriverPs3.h"

namespace traktor
{
	namespace input
	{

extern "C" void __module__Traktor_Input_Ps3()
{
	T_FORCE_LINK_REF(InputDriverPs3);
}

	}
}

#endif
