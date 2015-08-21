#ifndef traktor_telemetry_TelemetryClassFactory_H
#define traktor_telemetry_TelemetryClassFactory_H

#include "Core/Class/IRuntimeClassFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TELEMETRY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace telemetry
	{

class T_DLLCLASS TelemetryClassFactory : public IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(IRuntimeClassRegistrar* registrar) const;
};

	}
}

#endif	// traktor_telemetry_TelemetryClassFactory_H
