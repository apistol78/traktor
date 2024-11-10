/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Buffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/SystemMeshFactory.h"

namespace traktor::render
{
	namespace
	{

class InternalBuffer : public Buffer
{
public:
	explicit InternalBuffer(uint32_t bufferSize)
	:	Buffer(bufferSize)
	,	m_data(bufferSize)
	{
	}

	virtual void destroy() override final {}
	virtual void* lock() override final { return &m_data[0]; }
	virtual void unlock() override final {}
	virtual const IBufferView* getBufferView() const override final { return nullptr; }

private:
	AlignedVector< uint8_t > m_data;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SystemMeshFactory", SystemMeshFactory, MeshFactory)

Ref< Mesh > SystemMeshFactory::createMesh(
	const AlignedVector< VertexElement >& vertexElements,
	uint32_t vertexBufferSize,
	IndexType indexType,
	uint32_t indexBufferSize,
	const SmallMap< FourCC, uint32_t >& auxBufferSizes
) const
{
	Ref< Buffer > vertexBuffer;
	Ref< Buffer > indexBuffer;
	Ref< Buffer > auxBuffer;

	if (vertexBufferSize > 0)
	{
		const uint32_t vertexSize = getVertexSize(vertexElements);
		if (vertexSize == 0)
			return nullptr;
		vertexBuffer = new InternalBuffer(vertexBufferSize);
	}

	if (indexBufferSize > 0)
		indexBuffer = new InternalBuffer(indexBufferSize);

	SmallMap< FourCC, Ref< Buffer > > auxBuffers;
	for (auto aux : auxBufferSizes)
		auxBuffers[aux.first] = new InternalBuffer(aux.second);

	Ref< Mesh > mesh = new Mesh();
	mesh->setVertexElements(vertexElements);
	mesh->setVertexBuffer(vertexBuffer);
	mesh->setIndexType(indexType);
	mesh->setIndexBuffer(indexBuffer);
	mesh->setAuxBuffers(auxBuffers);
	return mesh;
}

}
