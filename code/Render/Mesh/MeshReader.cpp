#include "Render/Mesh/MeshReader.h"
#include "Render/Mesh/MeshFactory.h"
#include "Render/Mesh/Mesh.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Core/Math/Half.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.MeshReader", MeshReader, Object)

MeshReader::MeshReader(MeshFactory* meshFactory)
:	m_meshFactory(meshFactory)
{
}

Ref< Mesh > MeshReader::read(Stream* stream) const
{
	Reader reader(stream);

	uint32_t version;
	reader >> version;
	if (version != 3)
		return 0;

	uint32_t vertexElementCount;
	reader >> vertexElementCount;

	std::vector< VertexElement > vertexElements;
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

#if defined(T_LITTLE_ENDIAN)
		reader.read(vertex, vertexBufferSize);
#else
		for (unsigned int i = 0; i < vertexBufferSize; )
		{
			for (std::vector< VertexElement >::iterator j = vertexElements.begin(); j != vertexElements.end(); ++j)
			{
				switch (j->getDataType())
				{
				case DtFloat1:
					reader.read(&vertex[i + j->getOffset()], 1, sizeof(float));
					break;

				case DtFloat2:
					reader.read(&vertex[i + j->getOffset()], 2, sizeof(float));
					break;

				case DtFloat3:
					reader.read(&vertex[i + j->getOffset()], 3, sizeof(float));
					break;

				case DtFloat4:
					reader.read(&vertex[i + j->getOffset()], 4, sizeof(float));
					break;

				case DtByte4:
				case DtByte4N:
					reader.read(&vertex[i + j->getOffset()], 4, sizeof(uint8_t));
					break;

				case DtShort2:
				case DtShort2N:
					reader.read(&vertex[i + j->getOffset()], 2, sizeof(int16_t));
					break;

				case DtShort4:
				case DtShort4N:
					reader.read(&vertex[i + j->getOffset()], 4, sizeof(int16_t));
					break;

				case DtHalf2:
					reader.read(&vertex[i + j->getOffset()], 2, sizeof(half_t));
					break;

				case DtHalf4:
					reader.read(&vertex[i + j->getOffset()], 4, sizeof(half_t));
					break;
				}
			}
			i += getVertexSize(vertexElements);
		}
#endif

		mesh->getVertexBuffer()->unlock();
	}

	if (indexBufferSize > 0)
	{
		uint8_t* index = static_cast< uint8_t* >(mesh->getIndexBuffer()->lock());
		if (!index)
			return 0;

#if defined(T_LITTLE_ENDIAN)
		reader.read(index, indexBufferSize);
#else
		switch (indexType)
		{
		case ItUInt16:
			reader.read(index, indexBufferSize / sizeof(uint16_t), sizeof(uint16_t));
			break;

		case ItUInt32:
			reader.read(index, indexBufferSize / sizeof(uint32_t), sizeof(uint32_t));
			break;
		}
#endif

		mesh->getIndexBuffer()->unlock();
	}

	uint32_t partCount;
	reader >> partCount;

	std::vector< Mesh::Part > parts;
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

	Aabb boundingBox;
	boundingBox.mn.set(ext[0], ext[1], ext[2]);
	boundingBox.mx.set(ext[3], ext[4], ext[5]);
	mesh->setBoundingBox(boundingBox);

	return mesh;
}

	}
}
