/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/Editor/MeshVertexWriter.h"
#include "Core/Math/Half.h"

namespace traktor
{
	namespace mesh
	{

uint32_t writeVertexData(const AlignedVector< render::VertexElement >& vertexElements, uint8_t* vertex, render::DataUsage usage, uint32_t index, const float* data)
{
	AlignedVector< render::VertexElement >::const_iterator i = render::findVertexElement(vertexElements, usage, index);
	if (i == vertexElements.end())
		return 0;

	vertex += i->getOffset();

	uint32_t elementCount = render::getDataElementCount(i->getDataType());
	for (uint32_t j = 0; j < elementCount; ++j)
	{
		switch (i->getDataType())
		{
		case render::DtFloat1:
		case render::DtFloat2:
		case render::DtFloat3:
		case render::DtFloat4:
			*reinterpret_cast< float* >(vertex) = *data++;
			vertex += sizeof(float);
			break;

		case render::DtByte4:
		case render::DtByte4N:
			*vertex++ = uint8_t(255.0f * *data++);
			break;

		case render::DtShort2:
		case render::DtShort4:
		case render::DtShort2N:
		case render::DtShort4N:
			*reinterpret_cast< uint16_t* >(vertex) = uint16_t(65535.0f * *data++);
			vertex += sizeof(uint16_t);
			break;

		case render::DtHalf2:
		case render::DtHalf4:
			*reinterpret_cast< half_t* >(vertex) = floatToHalf(*data++);
			vertex += sizeof(half_t);
			break;

		default:
			T_FATAL_ERROR;
		}
	}

	return i->getSize();
}

uint32_t writeVertexData(const AlignedVector< render::VertexElement >& vertexElements, uint8_t* vertex, render::DataUsage usage, uint32_t index, const Vector2& data)
{
	float d[] = { data.x, data.y, 0.0f, 0.0f };
	return writeVertexData(vertexElements, vertex, usage, index, d);
}

uint32_t writeVertexData(const AlignedVector< render::VertexElement >& vertexElements, uint8_t* vertex, render::DataUsage usage, uint32_t index, const Vector4& data)
{
	float d[] = { data.x(), data.y(), data.z(), data.w() };
	return writeVertexData(vertexElements, vertex, usage, index, d);
}

	}
}
