/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
