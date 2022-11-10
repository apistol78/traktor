/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/Provider/IAchievementsProvider.h"

namespace traktor
{
	namespace online
	{

class LanAchievements : public IAchievementsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) override final;

	virtual bool set(const std::wstring& achievementId, bool reward) override final;
};

	}
}

