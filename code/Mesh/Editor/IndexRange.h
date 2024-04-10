/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor::mesh
{

struct IndexRange
{
	int32_t offsetFirst = 0;
	int32_t offsetLast = 0;
	int32_t minIndex = 0;
	int32_t maxIndex = 0;
	bool opaque = true;

	void mergeInto(AlignedVector< IndexRange >& ranges) const;

	bool operator < (const IndexRange& rh) const;
};

}
