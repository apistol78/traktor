/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/Lan/LanStatistics.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LanStatistics", LanStatistics, IStatisticsProvider)

bool LanStatistics::enumerate(std::map< std::wstring, int32_t >& outStats)
{
	return true;
}

bool LanStatistics::set(const std::wstring& statId, int32_t value)
{
	return true;
}

	}
}
