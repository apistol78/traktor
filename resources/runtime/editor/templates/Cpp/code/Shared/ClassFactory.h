#ifndef ClassFactory_H
#define ClassFactory_H

#include <Core/Class/IRuntimeClassFactory.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MYCPPTEST_SHARED_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class T_DLLCLASS ClassFactory : public traktor::IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(traktor::IRuntimeClassRegistrar* registrar) const T_OVERRIDE T_FINAL;
};

#endif	// ClassFactory_H
