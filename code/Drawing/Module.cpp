#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Drawing/DrawingClassFactory.h"

namespace traktor
{
	namespace drawing
	{

extern "C" void __module__Traktor_Drawing()
{
	T_FORCE_LINK_REF(DrawingClassFactory);
}

	}
}

#endif
