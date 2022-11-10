/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#if defined(T_STATIC)
#	include "Core/Class/BoxedClassFactory.h"
#	include "Core/Class/CoreClassFactory1.h"
#	include "Core/Class/CoreClassFactory2.h"
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
	T_FORCE_LINK_REF(BoxedClassFactory);
	T_FORCE_LINK_REF(CoreClassFactory1);
	T_FORCE_LINK_REF(CoreClassFactory2);
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
