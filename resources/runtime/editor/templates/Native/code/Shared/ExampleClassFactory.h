#pragma once

#include <Core/Class/IRuntimeClassFactory.h>

class T_DLLEXPORT ExampleClassFactory : public traktor::IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(traktor::IRuntimeClassRegistrar* registrar) const override final;
};
