/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Resource/ResourceClassFactory.h"

namespace traktor
{
	namespace resource
	{

extern "C" void __module__Traktor_Resource()
{
	T_FORCE_LINK_REF(ResourceClassFactory);
}

	}
}

#endif
