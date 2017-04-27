/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Video/VideoClassFactory.h"

namespace traktor
{
	namespace video
	{

extern "C" void __module__Traktor_Video()
{
	T_FORCE_LINK_REF(VideoClassFactory);
}

	}
}

#endif
