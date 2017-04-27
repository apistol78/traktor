/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Ui/UiClassFactory.h"

namespace traktor
{
	namespace ui
	{

extern "C" void __module__Traktor_Ui()
{
	T_FORCE_LINK_REF(UiClassFactory);
}

	}
}

#endif
