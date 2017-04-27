/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Flash/FlashClassFactory.h"
#	include "Flash/Action/Avm1/ActionVMImage1.h"
#	include "Flash/Action/Avm2/ActionVMImage2.h"

namespace traktor
{
	namespace flash
	{

extern "C" void __module__Traktor_Flash()
{
	T_FORCE_LINK_REF(FlashClassFactory);
	T_FORCE_LINK_REF(ActionVMImage1);
	T_FORCE_LINK_REF(ActionVMImage2);
}

	}
}

#endif
