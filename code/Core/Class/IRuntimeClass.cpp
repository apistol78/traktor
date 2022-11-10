/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
		return nullptr;

	Any retval = constructorDisptach->invoke(self, argc, argv);
	if (!retval.isObject())
		return nullptr;

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
	return nullptr;
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
			return runtimeClass->getPropertyName(i);
	}
	return "";
}

}
