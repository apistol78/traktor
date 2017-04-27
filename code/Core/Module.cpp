/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Core/Class/CoreClassFactory.h"
#	include "Core/Settings/PropertyArray.h"
#	include "Core/Settings/PropertyBoolean.h"
#	include "Core/Settings/PropertyColor.h"
#	include "Core/Settings/PropertyFloat.h"
#	include "Core/Settings/PropertyGroup.h"
#	include "Core/Settings/PropertyInteger.h"
#	include "Core/Settings/PropertyObject.h"
#	include "Core/Settings/PropertyString.h"
#	include "Core/Settings/PropertyStringArray.h"
#	include "Core/Settings/PropertyStringSet.h"

namespace traktor
{

extern "C" void __module__Traktor_Core()
{
	T_FORCE_LINK_REF(CoreClassFactory);
	T_FORCE_LINK_REF(PropertyArray);
	T_FORCE_LINK_REF(PropertyBoolean);
	T_FORCE_LINK_REF(PropertyColor);
	T_FORCE_LINK_REF(PropertyFloat);
	T_FORCE_LINK_REF(PropertyGroup);
	T_FORCE_LINK_REF(PropertyInteger);
	T_FORCE_LINK_REF(PropertyObject);
	T_FORCE_LINK_REF(PropertyString);
	T_FORCE_LINK_REF(PropertyStringArray);
	T_FORCE_LINK_REF(PropertyStringSet);
}

}

#endif
