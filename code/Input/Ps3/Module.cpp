/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Input/Ps3/InputDriverPs3.h"

namespace traktor
{
	namespace input
	{

extern "C" void __module__Traktor_Input_Ps3()
{
	T_FORCE_LINK_REF(InputDriverPs3);
}

	}
}

#endif
