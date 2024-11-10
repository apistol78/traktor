/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Misc/Endian.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshFactory.h"
#include "Render/Mesh/MeshReader.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.MeshReader", MeshReader, Object)

MeshReader::MeshReader(const MeshFactory* meshFactory)
:	m_meshFactory(meshFactory)
{
}

Ref< Mesh > MeshReader::read(IStream* stream) const
{
	Reader reader(stream);

	uint32_t version;
	reader >> version;
	if (version != 7)
		return nullptr;

	// Read vertex declaration.
	uint32_t vertexElementCount;
	reader >> vertexElementCount;

	AlignedVector< VertexElement > vertexElements;
	vertexElements.reserve(vertexElementCount);
	for (uint32_t i = 0; i < vertexElementCount; ++i)
	{
		uint32_t usage, type, offset, index;
		reader >> usage;
		reader >> type;
		reader >> offset;
		reader >> index;
		vertexElements.push_back(VertexElement(
			(DataUsage)usage,
			(DataType)type,
			offset,
			index
		));
	}

	// Read vertex buffer size.
	uint32_t vertexBufferSize;
	reader >> vertexBufferSize;

	// Read index declaration.
	uint32_t indexType;
	reader >> indexType;

	// Read index buffer size.
	uint32_t indexBufferSize;
	reader >> indexBufferSize;

	// Read aux buffer sizes.
	uint32_t auxBufferCount;
	reader >> auxBufferCount;

	SmallMap< FourCC, uint32_t > auxBufferSizes;
	for (uint32_t i = 0; i < auxBufferCount; ++i)
	{
		uint32_t id, size;
		reader >> id;
		reader >> size;
		auxBufferSizes[FourCC(id)] = size;
	}

	// Create mesh and buffers.
	Ref< Mesh > mesh = m_meshFactory->createMesh(
		vertexElements,
		vertexBufferSize,
		(IndexType)indexType,
		indexBufferSize,
		auxBufferSizes
	);
	if (!mesh)
		return nullptr;

	if (vertexBufferSize > 0)
	{
		uint8_t* vertex = static_cast< uint8_t* >(mesh->getVertexBuffer()->lock());
		if (!vertex)
			return nullptr;
		reader.read(vertex, vertexBufferSize);
		mesh->getVertexBuffer()->unlock();
	}

	if (indexBufferSize > 0)
	{
		uint8_t* index = static_cast< uint8_t* >(mesh->getIndexBuffer()->lock());
		if (!index)
			return nullptr;
		reader.read(index, indexBufferSize);
		mesh->getIndexBuffer()->unlock();
	}

	for (auto aux : auxBufferSizes)
	{
		uint8_t* ptr = static_cast< uint8_t* >(mesh->getAuxBuffer(aux.first)->lock());
		if (!ptr)
			return nullptr;
		reader.read(ptr, aux.second);
		mesh->getAuxBuffer(aux.first)->unlock();
	}

	uint32_t partCount;
	reader >> partCount;

	AlignedVector< Mesh::Part > parts;
	parts.resize(partCount);

	int32_t primitiveType;
	for (uint32_t i = 0; i < partCount; ++i)
	{
		reader >> parts[i].name;
		reader >> primitiveType; parts[i].primitives.type = PrimitiveType(primitiveType);
		reader >> parts[i].primitives.offset;
		reader >> parts[i].primitives.count;
		reader >> parts[i].primitives.indexed;
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
