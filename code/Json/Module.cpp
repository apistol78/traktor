/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Json/JsonClassFactory.h"
#	include "Json/JsonDocument.h"

namespace traktor
{
	namespace json
	{

extern "C" void __module__Traktor_Json()
{
	T_FORCE_LINK_REF(JsonClassFactory);
	T_FORCE_LINK_REF(JsonDocument);
}

	}
}

#endif
