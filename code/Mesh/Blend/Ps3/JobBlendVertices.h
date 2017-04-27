/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_JobBlendVertices_H
#define traktor_mesh_JobBlendVertices_H

#include <cell/spurs/job_descriptor.h>
#include "Render/Types.h"

namespace traktor
{
	namespace mesh
	{
		
struct JobBlendVertices
{
	CellSpursJobHeader header;

	struct BlendData
	{
		uintptr_t sourceVertices;
		uintptr_t destinationVertices;
		uint32_t vertexCount;
		float blendWeights[8];
		uint32_t blendWeightsCount;
		render::VertexElement vertexElements[8];
		uint32_t vertexElementsCount;
	}
	data;

	uint8_t pad[256 - sizeof(CellSpursJobHeader) - sizeof(BlendData)];
};

	}
}

#endif	// traktor_mesh_JobBlendVertices_H
