/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/IRuntimeClass.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.IRuntimeClass", IRuntimeClass, Object)

uint32_t findRuntimeClassMethodId(const IRuntimeClass* runtimeClass, const std::string& methodName)
{
	uint32_t methodCount = runtimeClass->getMethodCount();
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		if (runtimeClass->getMethodName(i) == methodName)
			return i;
	}
	return ~0U;
}

}
