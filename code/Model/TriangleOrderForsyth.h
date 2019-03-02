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

