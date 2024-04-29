/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Spark/Runtime/RuntimePlugin.h"
#	include "Spark/Runtime/SparkClassFactory.h"
#	include "Spark/Runtime/SparkLayerData.h"

namespace traktor::spark
{

extern "C" void __module__Traktor_Spark_Runtime()
{
	T_FORCE_LINK_REF(SparkClassFactory);
	T_FORCE_LINK_REF(SparkLayerData);
	T_FORCE_LINK_REF(RuntimePlugin);
}

}

#endif
