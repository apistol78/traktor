/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Mesh/Editor/IndexRange.h"

namespace traktor
{
	namespace mesh
	{

IndexRange::IndexRange()
:	offsetFirst(0)
,	offsetLast(0)
,	minIndex(0)
,	maxIndex(0)
,	opaque(true)
{
}

void IndexRange::mergeInto(std::vector< IndexRange >& ranges) const
{
	ranges.push_back(*this);

	// Sort ranges to ensure they are butt to head.
	std::sort(ranges.begin(), ranges.end());

	// Merge touching ranges.
	for (uint32_t i = 0; i < ranges.size() - 1; )
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
}
