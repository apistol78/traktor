/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Input/Android/InputAndroidClassFactory.h"
#	include "Input/Android/InputDriverAndroid.h"

namespace traktor
{
	namespace input
	{

extern "C" void __module__Traktor_Input_Android()
{
	T_FORCE_LINK_REF(InputAndroidClassFactory);
	T_FORCE_LINK_REF(InputDriverAndroid);
}

	}
}

#endif
