/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_TriangleOrderForsyth_H
#define traktor_model_TriangleOrderForsyth_H

#include "Core/Config.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace model
	{

void optimizeFaces(const AlignedVector< uint32_t >& indexList, uint32_t vertexCount, AlignedVector< uint32_t >& outNewIndexList, uint32_t lruCacheSize);

	}
}

#endif	// traktor_model_TriangleOrderForsyth_H
