#pragma once

#include "Core/Config.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace mesh
	{

struct IndexRange
{
	int32_t offsetFirst;
	int32_t offsetLast;
	int32_t minIndex;
	int32_t maxIndex;
	bool opaque;

	IndexRange();

	void mergeInto(AlignedVector< IndexRange >& ranges) const;

	bool operator < (const IndexRange& rh) const;
};

	}
}

