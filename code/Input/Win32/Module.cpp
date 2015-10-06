#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Input/Win32/InputDriverWin32.h"

namespace traktor
{
	namespace input
	{

extern "C" void __module__Traktor_Input_Win32()
{
	T_FORCE_LINK_REF(InputDriverWin32);
}

	}
}

#endif
