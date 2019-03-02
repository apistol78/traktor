#pragma once

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

