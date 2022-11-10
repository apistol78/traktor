/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <atomic>
#include "Spark/Types.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

std::atomic< int32_t > s_tag(0);

		}

int32_t allocateCacheTag()
{
	return ++s_tag;
}

	}
}
