/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/Provider/IStatisticsProvider.h"

namespace traktor
{
	namespace online
	{

class GcStatistics : public IStatisticsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, int32_t >& outStats) override final;

	virtual bool set(const std::wstring& statId, int32_t value) override final;
};

	}
}

