/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

namespace traktor
{

/*! Waitable object interface.
 * \ingroup Core
 */
class IWaitable
{
public:
	virtual ~IWaitable() {}

	virtual bool wait(int32_t timeout = -1) = 0;
};

}

