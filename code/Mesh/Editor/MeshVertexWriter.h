/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_MeshVertexWriter_H
#define traktor_mesh_MeshVertexWriter_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Render/VertexElement.h"

namespace traktor
{
	namespace mesh
	{

uint32_t writeVertexData(const AlignedVector< render::VertexElement >& vertexElements, uint8_t* vertex, render::DataUsage usage, uint32_t index, const float* data);

uint32_t writeVertexData(const AlignedVector< render::VertexElement >& vertexElements, uint8_t* vertex, render::DataUsage usage, uint32_t index, const Vector2& data);

uint32_t writeVertexData(const AlignedVector< render::VertexElement >& vertexElements, uint8_t* vertex, render::DataUsage usage, uint32_t index, const Vector4& data);

	}
}

#endif	// traktor_mesh_MeshVertexWriter_H
