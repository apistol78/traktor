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
#	include "Amalgam/Game/GameClassFactory.h"
#	include "Amalgam/Game/Engine/AudioLayerData.h"
#	include "Amalgam/Game/Engine/RuntimePlugin.h"
#	include "Amalgam/Game/Engine/StageData.h"
#	include "Amalgam/Game/Engine/VideoLayerData.h"
#	include "Amalgam/Game/Engine/WorldLayerData.h"

namespace traktor
{
	namespace amalgam
	{

extern "C" void __module__Traktor_Amalgam_Game()
{
	T_FORCE_LINK_REF(GameClassFactory);
	T_FORCE_LINK_REF(AudioLayerData);
	T_FORCE_LINK_REF(RuntimePlugin);
	T_FORCE_LINK_REF(StageData);
	T_FORCE_LINK_REF(VideoLayerData);
	T_FORCE_LINK_REF(WorldLayerData);
}

	}
}

#endif
