#ifndef traktor_IRuntimeClassRegistrar_H
#define traktor_IRuntimeClassRegistrar_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

class T_DLLCLASS IRuntimeClassRegistrar
{
public:
	virtual ~IRuntimeClassRegistrar() {}

	virtual void registerClass(IRuntimeClass* runtimeClass) = 0;
};

}

#endif	// traktor_IRuntimeClassRegistrar_H
