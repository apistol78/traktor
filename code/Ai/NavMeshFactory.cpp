/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <DetourNavMesh.h>
#include "Ai/NavMesh.h"
#include "Ai/NavMeshFactory.h"
#include "Ai/NavMeshResource.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Database/Database.h"
#include "Database/Instance.h"

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
	return makeTypeInfoSet< NavMesh >();
}

bool NavMeshFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > NavMeshFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< NavMesh > outputNavMesh = new NavMesh();

	Ref< const NavMeshResource > resource = instance->getObject< NavMeshResource >();
	if (!resource)
		return nullptr;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return nullptr;

	Reader r(stream);

	uint8_t version;
	r >> version;
	if (version != 3)
		return nullptr;

	dtNavMeshParams params;
	r >> params.orig[0];
	r >> params.orig[1];
	r >> params.orig[2];
	r >> params.tileWidth;
	r >> params.tileHeight;
	r >> params.maxTiles;
	r >> params.maxPolys;

	outputNavMesh->m_navMesh = dtAllocNavMesh();
	if (!outputNavMesh->m_navMesh)
		return nullptr;

	if (dtStatusFailed(outputNavMesh->m_navMesh->init(&params)))
		return nullptr;

	int32_t tileCount;
	r >> tileCount;

	for (int32_t i = 0; i < tileCount; ++i)
	{
		int32_t tileDataSize;
		r >> tileDataSize;
		if (tileDataSize <= 0)
			return nullptr;

		uint8_t* tileData = (uint8_t*)dtAlloc(tileDataSize, DT_ALLOC_PERM);
		if (!tileData)
			return nullptr;

		if (
			stream->read(tileData, tileDataSize) != tileDataSize ||
			dtStatusFailed(outputNavMesh->m_navMesh->addTile(tileData, tileDataSize, DT_TILE_FREE_DATA, 0, nullptr))
		)
		{
			dtFree(tileData);
			return nullptr;
		}
	}

	stream->close();
	stream = nullptr;

	return outputNavMesh;
}

void NavMeshFactory::destroy(Object* resource) const
{
}

}
