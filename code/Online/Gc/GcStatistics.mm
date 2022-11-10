/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/Gc/GcStatistics.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcStatistics", GcStatistics, IStatisticsProvider)

bool GcStatistics::enumerate(std::map< std::wstring, int32_t >& outStats)
{
	return false;
}

bool GcStatistics::set(const std::wstring& statId, int32_t value)
{
	return false;
}

	}
}
