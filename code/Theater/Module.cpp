/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Theater/TheaterControllerData.h"

namespace traktor
{
	namespace theater
	{

extern "C" void __module__Traktor_Theater()
{
	T_FORCE_LINK_REF(TheaterControllerData);
}

	}
}

#endif
