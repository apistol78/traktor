#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Spark/Runtime/RuntimePlugin.h"
#	include "Spark/Runtime/SparkClassFactory.h"
#	include "Spark/Runtime/SparkLayerData.h"

namespace traktor
{
	namespace spark
	{

extern "C" void __module__Traktor_Spark_Runtime()
{
	T_FORCE_LINK_REF(SparkClassFactory);
	T_FORCE_LINK_REF(SparkLayerData);
	T_FORCE_LINK_REF(RuntimePlugin);
}

	}
}

#endif
