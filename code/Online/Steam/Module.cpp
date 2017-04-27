/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Online/Steam/SteamGameConfiguration.h"
#	include "Online/Steam/SteamSessionManager.h"

namespace traktor
{
	namespace online
	{

extern "C" void __module__Traktor_Online_Steam()
{
	T_FORCE_LINK_REF(SteamGameConfiguration);
	T_FORCE_LINK_REF(SteamSessionManager);
}

	}
}

#endif
