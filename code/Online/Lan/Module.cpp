/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Online/Lan/LanGameConfiguration.h"
#	include "Online/Lan/LanSessionManager.h"

namespace traktor
{
	namespace online
	{

extern "C" void __module__Traktor_Online_Lan()
{
	T_FORCE_LINK_REF(LanGameConfiguration);
	T_FORCE_LINK_REF(LanSessionManager);
}

	}
}

#endif
