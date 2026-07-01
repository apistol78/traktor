/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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

	writer << (uint32_t)9;

	// Write vertex buffer.
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

	// Write index buffer.
	IndexType indexType = IndexType::UInt16;
	uint32_t indexBufferSize = 0;
	if (mesh->getIndexBuffer())
	{
		indexType = mesh->getIndexType();
		indexBufferSize = mesh->getIndexBuffer()->getBufferSize();
	}

	writer << (uint32_t)indexType;
	writer << indexBufferSize;

	// Write auxiliary buffers.
	writer << (uint32_t)mesh->getAuxBuffers().size();
	for (const auto aux : mesh->getAuxBuffers())
	{
		Ref< Buffer > auxBuffer = aux.second;
		if (!auxBuffer)
			return false;

		writer << (uint32_t)aux.first;
		writer << (uint32_t)auxBuffer->getBufferSize();
	}

	// Write buffer data.
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

	for (const auto aux : mesh->getAuxBuffers())
	{
		const uint32_t auxBufferSize = aux.second->getBufferSize();
		uint8_t* ptr = static_cast< uint8_t* >(aux.second->lock());
		writer.write(ptr, auxBufferSize);
		aux.second->unlock();
	}

	// Write primitives.
	{
		const AlignedVector< Primitives >& primitives = mesh->getPrimitives();

		const uint32_t primitivesCount = (uint32_t)primitives.size();
		writer << primitivesCount;

		for (unsigned int i = 0; i < primitivesCount; ++i)
		{
			writer << (int32_t)primitives[i].type;
			writer << primitives[i].offset;
			writer << primitives[i].count;
			writer << primitives[i].indexed;
		}
	}
	{
		const AlignedVector< RaytracingPrimitives >& rtp = mesh->getRaytracingPrimitives();

		const uint32_t rtpCount = (uint32_t)rtp.size();
		writer << rtpCount;

		for (unsigned int i = 0; i < rtpCount; ++i)
		{
			writer << (int32_t)rtp[i].primitives.type;
			writer << rtp[i].primitives.offset;
			writer << rtp[i].primitives.count;
			writer << rtp[i].primitives.indexed;
			writer << rtp[i].opaque;
		}
	}

	// Write bounding box.
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
