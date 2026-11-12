/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshFactory.h"

#include "Ai/NavMesh.h"
#include "Ai/NavMeshChunk.h"
#include "Ai/NavMeshResource.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Database/Database.h"
#include "Database/Instance.h"

#include <DetourNavMesh.h>

namespace traktor::ai
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshFactory", 0, NavMeshFactory, resource::IResourceFactory)

bool NavMeshFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet NavMeshFactory::getResourceTypes() const
{
	return makeTypeInfoSet< NavMeshResource >();
}

const TypeInfoSet NavMeshFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< NavMesh, NavMeshChunk >();
}

bool NavMeshFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > NavMeshFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const NavMeshResource > resource = instance->getObject< NavMeshResource >();
	if (!resource)
		return nullptr;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return nullptr;

	Reader r(stream);

	uint8_t version;
	r >> version;

	// Tiles of a chunk of a chunked navigation mesh.
	if (&productType == &type_of< NavMeshChunk >())
	{
		if (version != 1)
			return nullptr;

		Ref< NavMeshChunk > outputChunk = readTiles(r);
		stream->close();
		return outputChunk;
	}

	if (version != 3 && version != 4 && version != 5)
		return nullptr;

	Ref< NavMesh > outputNavMesh = new NavMesh();

	r >> outputNavMesh->m_origin[0];
	r >> outputNavMesh->m_origin[1];
	r >> outputNavMesh->m_origin[2];
	r >> outputNavMesh->m_tileWidth;
	r >> outputNavMesh->m_tileHeight;

	if (version >= 5)
	{
		int32_t chunkTiles;
		r >> chunkTiles;
		r >> outputNavMesh->m_maxPolys;

		// As many tiles as fit beside polygon index and 10 salt bits in a 32 bit polygon reference.
		int32_t polyBits = 0;
		while ((1 << polyBits) < outputNavMesh->m_maxPolys)
			++polyBits;
		outputNavMesh->m_maxTiles = 1 << (22 - polyBits);
		outputNavMesh->m_chunkWidth = outputNavMesh->m_tileWidth * chunkTiles;

		int32_t chunkCount;
		r >> chunkCount;

		outputNavMesh->m_chunks.resize(chunkCount);
		for (auto& chunk : outputNavMesh->m_chunks)
		{
			uint8_t id[16];
			r >> chunk.x;
			r >> chunk.z;
			if (r.read(id, sizeof(id)) != sizeof(id))
				return nullptr;
			chunk.id = Guid(id);
		}
	}
	else
	{
		r >> outputNavMesh->m_maxTiles;
		r >> outputNavMesh->m_maxPolys;

		outputNavMesh->m_tiles = readTiles(r);
		if (!outputNavMesh->m_tiles)
			return nullptr;
	}

	if (version >= 4)
	{
		int32_t modifierCount;
		r >> modifierCount;

		for (int32_t i = 0; i < modifierCount; ++i)
		{
			uint8_t id[16];
			if (r.read(id, sizeof(id)) != sizeof(id))
				return nullptr;

			NavMesh::Modifier& modifier = outputNavMesh->m_modifiers[Guid(id)];
			r >> modifier.area;

			float bounds[6];
			for (int32_t j = 0; j < 6; ++j)
				r >> bounds[j];

			modifier.bounds = Aabb3(
				Vector4(bounds[0], bounds[1], bounds[2], 1.0f),
				Vector4(bounds[3], bounds[4], bounds[5], 1.0f));
		}
	}

	stream->close();
	stream = nullptr;

	return outputNavMesh;
}

void NavMeshFactory::destroy(Object* resource) const
{
}

Ref< NavMeshChunk > NavMeshFactory::readTiles(Reader& r) const
{
	int32_t tileCount;
	r >> tileCount;

	Ref< NavMeshChunk > chunk = new NavMeshChunk();
	chunk->m_tiles.resize(tileCount);
	for (auto& tile : chunk->m_tiles)
	{
		int32_t tileDataSize;
		r >> tileDataSize;
		if (tileDataSize <= 0)
			return nullptr;

		tile.resize(tileDataSize);
		if (r.read(tile.ptr(), tileDataSize) != tileDataSize)
			return nullptr;

		const dtMeshHeader* header = (const dtMeshHeader*)tile.c_ptr();
		chunk->m_bounds.contain(Vector4(header->bmin[0], header->bmin[1], header->bmin[2], 1.0f));
		chunk->m_bounds.contain(Vector4(header->bmax[0], header->bmax[1], header->bmax[2], 1.0f));
	}

	return chunk;
}

}
