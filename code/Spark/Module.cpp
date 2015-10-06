#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Spark/SparkClassFactory.h"

namespace traktor
{
	namespace spark
	{

extern "C" void __module__Traktor_Spark()
{
	T_FORCE_LINK_REF(SparkClassFactory);
}

	}
}

#endif
