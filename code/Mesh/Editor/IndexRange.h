#ifndef traktor_mesh_IndexRange_H
#define traktor_mesh_IndexRange_H

#include <vector>
#include "Core/Config.h"

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

	void mergeInto(std::vector< IndexRange >& ranges) const;

	bool operator < (const IndexRange& rh) const;
};

	}
}

#endif	// traktor_mesh_IndexRange_H
