/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Mesh/Editor/IndexRange.h"

namespace traktor::mesh
{

void IndexRange::mergeInto(AlignedVector< IndexRange >& ranges) const
{
	ranges.push_back(*this);

	// Sort ranges to ensure they are butt to head.
	std::sort(ranges.begin(), ranges.end());

	// Merge touching ranges.
	for (size_t i = 0; i < ranges.size() - 1; )
	{
		if (
			ranges[i].offsetLast == ranges[i + 1].offsetFirst &&
			ranges[i].opaque == ranges[i + 1].opaque
		)
		{
			ranges[i + 1].offsetFirst = ranges[i].offsetFirst;
			ranges[i + 1].minIndex = std::min(ranges[i + 1].minIndex, ranges[i].minIndex);
			ranges[i + 1].maxIndex = std::max(ranges[i + 1].maxIndex, ranges[i].maxIndex);
			ranges.erase(ranges.begin() + i);
		}
		else
			++i;
	}
}

bool IndexRange::operator < (const IndexRange& rh) const
{
	return offsetFirst < rh.offsetFirst;
}

}
