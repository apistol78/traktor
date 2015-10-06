#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Amalgam/Run/RunClassFactory.h"

namespace traktor
{
	namespace amalgam
	{

extern "C" void __module__Traktor_Amalgam_Run()
{
	T_FORCE_LINK_REF(RunClassFactory);
}

	}
}

#endif
