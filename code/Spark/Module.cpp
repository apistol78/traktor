#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Spark/ClassFactory.h"

namespace traktor
{
	namespace spark
	{

extern "C" void __module__Traktor_Spark()
{
	T_FORCE_LINK_REF(ClassFactory);
}

	}
}

#endif
