/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeDispatch.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.IRuntimeClass", IRuntimeClass, Object)

Ref< ITypedObject > createRuntimeClassInstance(const IRuntimeClass* runtimeClass, ITypedObject* self, uint32_t argc, const Any* argv)
{
	const IRuntimeDispatch* constructorDisptach = runtimeClass->getConstructorDispatch();
	if (!constructorDisptach)
		return 0;

	Any retval = constructorDisptach->invoke(self, argc, argv);
	if (!retval.isObject())
		return 0;

	return retval.getObjectUnsafe();
}

const IRuntimeDispatch* findRuntimeClassMethod(const IRuntimeClass* runtimeClass, const std::string& methodName)
{
	uint32_t methodCount = runtimeClass->getMethodCount();
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		if (runtimeClass->getMethodName(i) == methodName)
			return runtimeClass->getMethodDispatch(i);
	}
	return 0;
}

std::string findRuntimeClassMethodName(const IRuntimeClass* runtimeClass, const IRuntimeDispatch* methodDispatch)
{
	uint32_t methodCount = runtimeClass->getMethodCount();
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		if (runtimeClass->getMethodDispatch(i) == methodDispatch)
			return runtimeClass->getMethodName(i);
	}
	uint32_t staticMethodCount = runtimeClass->getStaticMethodCount();
	for (uint32_t i = 0; i < staticMethodCount; ++i)
	{
		if (runtimeClass->getStaticMethodDispatch(i) == methodDispatch)
			return runtimeClass->getStaticMethodName(i);
	}
	return "";
}

std::string findRuntimeClassPropertyName(const IRuntimeClass* runtimeClass, const IRuntimeDispatch* propertyDispatch)
{
	uint32_t propertyCount = runtimeClass->getPropertiesCount();
	for (uint32_t i = 0; i < propertyCount; ++i)
	{
		if (runtimeClass->getPropertyGetDispatch(i) == propertyDispatch || runtimeClass->getPropertySetDispatch(i) == propertyDispatch)
			return runtimeClass->getStaticMethodName(i);
	}
	return "";
}

}
