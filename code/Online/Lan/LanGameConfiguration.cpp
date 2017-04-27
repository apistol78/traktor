/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/Lan/LanGameConfiguration.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.online.LanGameConfiguration", 0, LanGameConfiguration, IGameConfiguration)

LanGameConfiguration::LanGameConfiguration()
{
}

void LanGameConfiguration::serialize(ISerializer& s)
{
}

	}
}
