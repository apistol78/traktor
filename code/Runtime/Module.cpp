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

#include "Runtime/GameClassFactory.h"
#include "Runtime/Engine/AudioLayerData.h"
#include "Runtime/Engine/RuntimePlugin.h"
#include "Runtime/Engine/ScreenLayerData.h"
#include "Runtime/Engine/VideoLayerData.h"
#include "Runtime/Engine/WorldLayerData.h"

namespace traktor
{
	namespace runtime
	{

extern "C" void __module__Traktor_Runtime()
{
	T_FORCE_LINK_REF(AudioLayerData);
	T_FORCE_LINK_REF(GameClassFactory);
	T_FORCE_LINK_REF(RuntimePlugin);
	T_FORCE_LINK_REF(ScreenLayerData);
	T_FORCE_LINK_REF(VideoLayerData);
	T_FORCE_LINK_REF(WorldLayerData);
}

	}
}

#endif
