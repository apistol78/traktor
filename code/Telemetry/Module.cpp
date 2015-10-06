#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#include "Telemetry/TelemetryClassFactory.h"

namespace traktor
{
	namespace telemetry
	{

extern "C" void __module__Traktor_Telemetry()
{
	T_FORCE_LINK_REF(TelemetryClassFactory);
}

	}
}

#endif
