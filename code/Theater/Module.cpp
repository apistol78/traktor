#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Theater/TheaterClassFactory.h"
#	include "Theater/TheaterControllerData.h"

namespace traktor
{
	namespace theater
	{

extern "C" void __module__Traktor_Theater()
{
	T_FORCE_LINK_REF(TheaterClassFactory);
	T_FORCE_LINK_REF(TheaterControllerData);
}

	}
}

#endif
