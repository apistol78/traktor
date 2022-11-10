/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class LanGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	LanGameConfiguration();

	virtual void serialize(ISerializer& s) override final;
};

	}
}

