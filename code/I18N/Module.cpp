/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "I18N/I18NClassFactory.h"

namespace traktor
{
	namespace i18n
	{

extern "C" void __module__Traktor_I18N()
{
	T_FORCE_LINK_REF(I18NClassFactory);
}

	}
}

#endif
