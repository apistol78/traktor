/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Misc/Endian.h"
#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/MeshFactory.h"
#include "Render/Mesh/Mesh.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.MeshReader", MeshReader, Object)

MeshReader::MeshReader(MeshFactory* meshFactory)
:	m_meshFactory(meshFactory)
{
}

Ref< Mesh > MeshReader::read(IStream* stream) const
{
	Reader reader(stream);

	uint32_t version;
	reader >> version;
	if (version != 3)
		return 0;

	uint32_t vertexElementCount;
	reader >> vertexElementCount;

	AlignedVector< VertexElement > vertexElements;
	vertexElements.reserve(vertexElementCount);

	for (uint32_t i = 0; i < vertexElementCount; ++i)
	{
		uint32_t usage;
		reader >> usage;

		uint32_t type;
		reader >> type;

		uint32_t offset;
		reader >> offset;

		uint32_t index;
		reader >> index;

		vertexElements.push_back(VertexElement(
			DataUsage(usage),
			DataType(type),
			offset,
			index
		));
	}

	uint32_t vertexBufferSize;
	reader >> vertexBufferSize;

	uint32_t indexType;
	reader >> indexType;

	uint32_t indexBufferSize;
	reader >> indexBufferSize;

	Ref< Mesh > mesh = m_meshFactory->createMesh(vertexElements, vertexBufferSize, IndexType(indexType), indexBufferSize);
	if (!mesh)
		return 0;

	if (vertexBufferSize > 0)
	{
		uint8_t* vertex = static_cast< uint8_t* >(mesh->getVertexBuffer()->lock());
		if (!vertex)
			return 0;

		reader.read(vertex, vertexBufferSize);

#if !defined(T_LITTLE_ENDIAN)
		uint32_t vertexSize = getVertexSize(vertexElements);
		for (uint32_t i = 0; i < vertexBufferSize; i += vertexSize)
		{
			for (AlignedVector< VertexElement >::iterator j = vertexElements.begin(); j != vertexElements.end(); ++j)
			{
				uint8_t* vertexElm = &vertex[i + j->getOffset()];
				switch (j->getDataType())
				{
				case DtFloat1:
					swap8in32(*(float*)vertexElm);
					break;

				case DtFloat2:
					swap8in32(*(float*)vertexElm);
					swap8in32(*(float*)(vertexElm + 4));
					break;

				case DtFloat3:
					swap8in32(*(float*)vertexElm);
					swap8in32(*(float*)(vertexElm + 4));
					swap8in32(*(float*)(vertexElm + 8));
					break;

				case DtFloat4:
					swap8in32(*(float*)vertexElm);
					swap8in32(*(float*)(vertexElm + 4));
					swap8in32(*(float*)(vertexElm + 8));
					swap8in32(*(float*)(vertexElm + 12));
					break;

				case DtShort2:
				case DtShort2N:
					swap8in32(*(uint16_t*)vertexElm);
					swap8in32(*(uint16_t*)(vertexElm + 2));
					break;

				case DtShort4:
				case DtShort4N:
					swap8in32(*(uint16_t*)vertexElm);
					swap8in32(*(uint16_t*)(vertexElm + 2));
					swap8in32(*(uint16_t*)(vertexElm + 4));
					swap8in32(*(uint16_t*)(vertexElm + 6));
					break;

				case DtHalf2:
					swap8in32(*(uint16_t*)vertexElm);
					swap8in32(*(uint16_t*)(vertexElm + 2));
					break;

				case DtHalf4:
					swap8in32(*(uint16_t*)vertexElm);
					swap8in32(*(uint16_t*)(vertexElm + 2));
					swap8in32(*(uint16_t*)(vertexElm + 4));
					swap8in32(*(uint16_t*)(vertexElm + 6));
					break;
				}
			}
		}
#endif

		mesh->getVertexBuffer()->unlock();
	}

	if (indexBufferSize > 0)
	{
		uint8_t* index = static_cast< uint8_t* >(mesh->getIndexBuffer()->lock());
		if (!index)
			return 0;

		reader.read(index, indexBufferSize);

#if !defined(T_LITTLE_ENDIAN)
		switch (indexType)
		{
		case ItUInt16:
			for (uint32_t i = 0; i < indexBufferSize; i += 2)
				swap8in32(*(uint16_t*)(index + i));
			break;

		case ItUInt32:
			for (uint32_t i = 0; i < indexBufferSize; i += 4)
				swap8in32(*(uint32_t*)(index + i));
			break;
		}
#endif

		mesh->getIndexBuffer()->unlock();
	}

	uint32_t partCount;
	reader >> partCount;

	AlignedVector< Mesh::Part > parts;
	parts.resize(partCount);

	for (uint32_t i = 0; i < partCount; ++i)
	{
		int32_t primitiveType;

		reader >> parts[i].name;
		reader >> primitiveType; parts[i].primitives.type = PrimitiveType(primitiveType);
		reader >> parts[i].primitives.indexed;
		reader >> parts[i].primitives.offset;
		reader >> parts[i].primitives.count;
		reader >> parts[i].primitives.minIndex;
		reader >> parts[i].primitives.maxIndex;
	}

	mesh->setParts(parts);
	
	float ext[6];
	reader >> ext[0];
	reader >> ext[1];
	reader >> ext[2];
	reader >> ext[3];
	reader >> ext[4];
	reader >> ext[5];

	Aabb3 boundingBox;
	boundingBox.mn.set(ext[0], ext[1], ext[2]);
	boundingBox.mx.set(ext[3], ext[4], ext[5]);
	mesh->setBoundingBox(boundingBox);

	return mesh;
}

	}
}
