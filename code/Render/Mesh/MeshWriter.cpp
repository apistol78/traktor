/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Mesh/MeshWriter.h"
#include "Render/Mesh/Mesh.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Core/Math/Half.h"
#include "Core/Io/Writer.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.MeshWriter", MeshWriter, Object)

bool MeshWriter::write(IStream* stream, const Mesh* mesh) const
{
	Writer writer(stream);

	writer << uint32_t(3);

	const AlignedVector< VertexElement >& vertexElements = mesh->getVertexElements();
	writer << uint32_t(vertexElements.size());

	for (AlignedVector< VertexElement >::const_iterator i = vertexElements.begin(); i != vertexElements.end(); ++i)
	{
		writer << uint32_t(i->getDataUsage());
		writer << uint32_t(i->getDataType());
		writer << uint32_t(i->getOffset());
		writer << uint32_t(i->getIndex());
	}

	uint32_t vertexBufferSize = 0;
	if (mesh->getVertexBuffer())
		vertexBufferSize = mesh->getVertexBuffer()->getBufferSize();
	
	writer << vertexBufferSize;

	IndexType indexType = ItUInt16;
	uint32_t indexBufferSize = 0;
	if (mesh->getIndexBuffer())
	{
		indexType = mesh->getIndexBuffer()->getIndexType();
		indexBufferSize = mesh->getIndexBuffer()->getBufferSize();
	}

	writer << uint32_t(indexType);
	writer << indexBufferSize;

	if (vertexBufferSize > 0)
	{
		uint8_t* vertex = static_cast< uint8_t* >(mesh->getVertexBuffer()->lock());
		for (uint32_t i = 0; i < vertexBufferSize; )
		{
			for (AlignedVector< VertexElement >::const_iterator j = vertexElements.begin(); j != vertexElements.end(); ++j)
			{
				switch (j->getDataType())
				{
				case DtFloat1:
					writer.write(&vertex[i + j->getOffset()], 1, sizeof(float));
					break;

				case DtFloat2:
					writer.write(&vertex[i + j->getOffset()], 2, sizeof(float));
					break;

				case DtFloat3:
					writer.write(&vertex[i + j->getOffset()], 3, sizeof(float));
					break;

				case DtFloat4:
					writer.write(&vertex[i + j->getOffset()], 4, sizeof(float));
					break;

				case DtByte4:
				case DtByte4N:
					writer.write(&vertex[i + j->getOffset()], 4, sizeof(uint8_t));
					break;

				case DtShort2:
				case DtShort2N:
					writer.write(&vertex[i + j->getOffset()], 2, sizeof(int16_t));
					break;

				case DtShort4:
				case DtShort4N:
					writer.write(&vertex[i + j->getOffset()], 4, sizeof(int16_t));
					break;

				case DtHalf2:
					writer.write(&vertex[i + j->getOffset()], 2, sizeof(half_t));
					break;

				case DtHalf4:
					writer.write(&vertex[i + j->getOffset()], 4, sizeof(half_t));
					break;
				}
			}
			i += getVertexSize(vertexElements);
		}
		mesh->getVertexBuffer()->unlock();
	}

	if (indexBufferSize > 0)
	{
		uint8_t* index = static_cast< uint8_t* >(mesh->getIndexBuffer()->lock());

		switch (indexType)
		{
		case ItUInt16:
			writer.write(index, indexBufferSize / sizeof(uint16_t), sizeof(uint16_t));
			break;

		case ItUInt32:
			writer.write(index, indexBufferSize / sizeof(uint32_t), sizeof(uint32_t));
			break;
		}

		mesh->getIndexBuffer()->unlock();
	}

	const AlignedVector< Mesh::Part >& parts = mesh->getParts();

	uint32_t partCount = uint32_t(parts.size());
	writer << partCount;

	for (unsigned int i = 0; i < partCount; ++i)
	{
		writer << parts[i].name;
		writer << int32_t(parts[i].primitives.type);
		writer << parts[i].primitives.indexed;
		writer << parts[i].primitives.offset;
		writer << parts[i].primitives.count;
		writer << parts[i].primitives.minIndex;
		writer << parts[i].primitives.maxIndex;
	}
	
	const Aabb3& boundingBox = mesh->getBoundingBox();
	writer << boundingBox.mn.x();
	writer << boundingBox.mn.y();
	writer << boundingBox.mn.z();
	writer << boundingBox.mx.x();
	writer << boundingBox.mx.y();
	writer << boundingBox.mx.z();

	return true;
}

	}
}
