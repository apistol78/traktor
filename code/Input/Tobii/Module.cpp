#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Input/Tobii/InputDriverTobii.h"

namespace traktor
{
	namespace input
	{

extern "C" void __module__Traktor_Input_Tobii()
{
	T_FORCE_LINK_REF(InputDriverTobii);
}

	}
}

#endif
