#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Ai/AiClassFactory.h"

namespace traktor
{
	namespace ai
	{

extern "C" void __module__Traktor_Ai()
{
	T_FORCE_LINK_REF(AiClassFactory);
}

	}
}

#endif
