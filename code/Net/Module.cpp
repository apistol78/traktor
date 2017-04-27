/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Net/NetClassFactory.h"

namespace traktor
{
	namespace net
	{

extern "C" void __module__Traktor_Net()
{
	T_FORCE_LINK_REF(NetClassFactory);
}

	}
}

#endif
