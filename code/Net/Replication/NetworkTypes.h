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
	namespace net
	{

enum { MaxDataSize = 1024 };
enum { MaxPeers = 32 };

typedef uint64_t net_handle_t;	//!< Globally unique handle.

	}
}

