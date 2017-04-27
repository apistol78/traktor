/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Ai/AiClassFactory.h"

namespace traktor
{
	namespace ai
	{

extern "C" void __module__Traktor_Ai()
{
	T_FORCE_LINK_REF(AiClassFactory);
}

	}
}

#endif
