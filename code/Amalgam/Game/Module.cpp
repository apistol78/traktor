/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Amalgam/Game/GameClassFactory.h"
#	include "Amalgam/Game/Engine/AudioLayerData.h"
#	include "Amalgam/Game/Engine/FlashLayerData.h"
#	include "Amalgam/Game/Engine/RuntimePlugin.h"
#	include "Amalgam/Game/Engine/SparkLayerData.h"
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
	T_FORCE_LINK_REF(FlashLayerData);
	T_FORCE_LINK_REF(RuntimePlugin);
	T_FORCE_LINK_REF(SparkLayerData);
	T_FORCE_LINK_REF(StageData);
	T_FORCE_LINK_REF(VideoLayerData);
	T_FORCE_LINK_REF(WorldLayerData);
}

	}
}

#endif
