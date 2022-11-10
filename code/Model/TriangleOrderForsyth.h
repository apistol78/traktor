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
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace model
	{

void optimizeFaces(const AlignedVector< uint32_t >& indexList, uint32_t vertexCount, AlignedVector< uint32_t >& outNewIndexList, uint32_t lruCacheSize);

	}
}

