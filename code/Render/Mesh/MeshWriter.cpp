/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Half.h"
#include "Core/Io/Writer.h"
#include "Render/Buffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshWriter.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.MeshWriter", MeshWriter, Object)

bool MeshWriter::write(IStream* stream, const Mesh* mesh) const
{
	Writer writer(stream);

	writer << (uint32_t)5;

	const auto& vertexElements = mesh->getVertexElements();
	writer << (uint32_t)vertexElements.size();

	for (const auto& vertexElement : vertexElements)
	{
		writer << (uint32_t)vertexElement.getDataUsage();
		writer << (uint32_t)vertexElement.getDataType();
		writer << (uint32_t)vertexElement.getOffset();
		writer << (uint32_t)vertexElement.getIndex();
	}

	const uint32_t vertexBufferSize = (mesh->getVertexBuffer() != nullptr) ? mesh->getVertexBuffer()->getBufferSize() : 0;
	writer << vertexBufferSize;

	IndexType indexType = IndexType::UInt16;
	uint32_t indexBufferSize = 0;
	if (mesh->getIndexBuffer())
	{
		indexType = mesh->getIndexType();
		indexBufferSize = mesh->getIndexBuffer()->getBufferSize();
	}

	writer << (uint32_t)indexType;
	writer << indexBufferSize;

	const uint32_t auxBufferSize = (mesh->getAuxBuffer() != nullptr) ? mesh->getAuxBuffer()->getBufferSize() : 0;
	writer << auxBufferSize;

	if (vertexBufferSize > 0)
	{
		uint8_t* vertex = static_cast< uint8_t* >(mesh->getVertexBuffer()->lock());
		for (uint32_t i = 0; i < vertexBufferSize; )
		{
			for (const auto& vertexElement : vertexElements)
			{
				switch (vertexElement.getDataType())
				{
				case DtFloat1:
					writer.write(&vertex[i + vertexElement.getOffset()], 1, sizeof(float));
					break;

				case DtFloat2:
					writer.write(&vertex[i + vertexElement.getOffset()], 2, sizeof(float));
					break;

				case DtFloat3:
					writer.write(&vertex[i + vertexElement.getOffset()], 3, sizeof(float));
					break;

				case DtFloat4:
					writer.write(&vertex[i + vertexElement.getOffset()], 4, sizeof(float));
					break;

				case DtByte4:
				case DtByte4N:
					writer.write(&vertex[i + vertexElement.getOffset()], 4, sizeof(uint8_t));
					break;

				case DtShort2:
				case DtShort2N:
					writer.write(&vertex[i + vertexElement.getOffset()], 2, sizeof(int16_t));
					break;

				case DtShort4:
				case DtShort4N:
					writer.write(&vertex[i + vertexElement.getOffset()], 4, sizeof(int16_t));
					break;

				case DtHalf2:
					writer.write(&vertex[i + vertexElement.getOffset()], 2, sizeof(half_t));
					break;

				case DtHalf4:
					writer.write(&vertex[i + vertexElement.getOffset()], 4, sizeof(half_t));
					break;

				default:
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
		case IndexType::UInt16:
			writer.write(index, indexBufferSize / sizeof(uint16_t), sizeof(uint16_t));
			break;

		case IndexType::UInt32:
			writer.write(index, indexBufferSize / sizeof(uint32_t), sizeof(uint32_t));
			break;

		default:
			break;
		}
		mesh->getIndexBuffer()->unlock();
	}

	if (auxBufferSize > 0)
	{
		uint8_t* aux = static_cast< uint8_t* >(mesh->getAuxBuffer()->lock());
		writer.write(aux, auxBufferSize);
		mesh->getAuxBuffer()->unlock();
	}

	const AlignedVector< Mesh::Part >& parts = mesh->getParts();

	const uint32_t partCount = (uint32_t)parts.size();
	writer << partCount;

	for (unsigned int i = 0; i < partCount; ++i)
	{
		writer << parts[i].name;
		writer << (int32_t)parts[i].primitives.type;
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
