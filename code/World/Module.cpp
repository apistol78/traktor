/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(T_STATIC)
#	include "World/WorldClassFactory.h"
#	include "World/Deferred/WorldRendererDeferred.h"
#	include "World/Forward/WorldRendererForward.h"
#	include "World/Simple/WorldRendererSimple.h"

namespace traktor::world
{

extern "C" void __module__Traktor_World()
{
	T_FORCE_LINK_REF(WorldClassFactory);
	T_FORCE_LINK_REF(WorldRendererDeferred);
	T_FORCE_LINK_REF(WorldRendererForward);
	T_FORCE_LINK_REF(WorldRendererSimple);
}

}
#endif
