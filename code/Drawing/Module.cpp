/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Drawing/DrawingClassFactory.h"

namespace traktor
{
	namespace drawing
	{

extern "C" void __module__Traktor_Drawing()
{
	T_FORCE_LINK_REF(DrawingClassFactory);
}

	}
}

#endif
