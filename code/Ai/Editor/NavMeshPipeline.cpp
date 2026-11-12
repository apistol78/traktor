/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/Editor/NavMeshPipeline.h"

#include "Ai/Editor/NavMeshAsset.h"
#include "Ai/Editor/NavMeshChunkAsset.h"
#include "Ai/NavMeshModifierEntityComponentData.h"
#include "Ai/NavMeshResource.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Editor/ConvertHeightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/Triangulate.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/OceanComponentData.h"
#include "Terrain/TerrainComponentData.h"
#include "World/Editor/IEntityReplicator.h"
#include "World/Editor/ResolveExternal.h"
#include "World/Editor/Traverser.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/VolumeComponentData.h"
#include "World/EntityData.h"

#include <atomic>
#include <cstring>
#include <DetourCommon.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <limits>
#include <memory>
#include <Recast.h>

namespace traktor::ai
{
namespace
{

const float c_oceanThreshold = 0.25f;
const float c_heightfieldMaxError = 0.05f;
const int32_t c_chunkMaxTilePolys = 256;	//!< Leaves 14 bits of a poly reference for tiles, room for 10 chunks of 40x40 tiles.
const Guid c_guidChunkSeed(L"{E3CE3750-CF25-4140-AD12-69220FA956C7}");

class BuildContext : public rcContext
{
protected:
	virtual void doLog(const rcLogCategory /*category*/, const char* msg, const int /*len*/)
	{
		T_DEBUG(mbstows(msg));
	}
};

struct NavMeshSourceModel
{
	Ref< const model::Model > model;
	Transform transform;
	Guid entityId;

	NavMeshSourceModel() = default;

	NavMeshSourceModel(const model::Model* model_, const Transform& transform_, const Guid& entityId_)
		: model(model_)
		, transform(transform_)
		, entityId(entityId_)
	{
	}
};

struct NavMeshModifier
{
	Guid id;
	Aabb3 bounds;
	uint8_t area;
};

struct NavMeshTerrain
{
	Guid terrain;
	Transform transform;
};

//! Geometry and settings gathered from source scene.
struct NavMeshSource
{
	AlignedVector< NavMeshSourceModel > models;
	AlignedVector< NavMeshTerrain > terrains;
	SmallMap< Guid, uint32_t > entityHashes;	//!< Hash of each entity with geometry, if requested.
	Aabb3 maximumBounds;
	float oceanHeight = -std::numeric_limits< float >::max();
	bool oceanClip = false;
};

//! Range of tiles to build in a grid of tiles.
struct NavMeshTileGrid
{
	float originX = 0.0f;		//!< Origin of tile 0, 0.
	float originZ = 0.0f;
	float tileWidth = 0.0f;
	float tileDepth = 0.0f;
	float minY = 0.0f;
	float maxY = 0.0f;
	int32_t firstX = 0;			//!< First tile to build.
	int32_t firstZ = 0;
	int32_t tilesX = 0;			//!< Number of tiles to build.
	int32_t tilesZ = 0;
};

void copyUnaligned3(float out[3], const Vector4& source)
{
	out[0] = source.x();
	out[1] = source.y();
	out[2] = source.z();
}

void collectModifierEntities(const ISerializable* object, const Guid& parentId, AlignedVector< std::pair< Guid, Ref< const world::EntityData > > >& outEntities)
{
	world::Traverser::visit(object, [&](const world::EntityData* entityData) -> world::Traverser::Result {
		if (!entityData->getState().visible)
			return world::Traverser::Result::Skip;

		// Undo id permutation by parent to recover entity's unresolved id.
		if (entityData->getComponent< NavMeshModifierEntityComponentData >())
			outEntities.push_back({ entityData->getId().permutation(parentId), entityData });

		collectModifierEntities(entityData, entityData->getId(), outEntities);
		return world::Traverser::Result::Skip;
	});
}

//! Heightfield instance and asset of terrain.
bool readTerrainHeightfield(editor::IPipelineBuilder* pipelineBuilder, const Guid& terrainId, Ref< db::Instance >& outInstance, Ref< const hf::HeightfieldAsset >& outAsset)
{
	Ref< const terrain::TerrainAsset > terrainAsset = pipelineBuilder->getObjectReadOnly< terrain::TerrainAsset >(terrainId);
	if (!terrainAsset)
		return false;

	outInstance = pipelineBuilder->getSourceDatabase()->getInstance(terrainAsset->getHeightfield());
	if (!outInstance)
		return false;

	outAsset = outInstance->getObject< const hf::HeightfieldAsset >();
	return outAsset != nullptr;
}

//! Range of heightfield tiles overlapping local bounds, or every tile if bounds are empty.
void getTerrainTileRange(const hf::HeightfieldAsset* heightfieldAsset, const Aabb3& localBounds, int32_t outRange[4])
{
	const Vector4& worldExtent = heightfieldAsset->getWorldExtent();
	const int32_t tileCount = std::max(heightfieldAsset->getTileCount(), 1);
	const float c_epsilon = 0.01f;

	outRange[0] = outRange[1] = 0;
	outRange[2] = outRange[3] = tileCount - 1;
	if (localBounds.empty())
		return;

	const float tileWidth = worldExtent.x() / tileCount;
	const float tileDepth = worldExtent.z() / tileCount;
	outRange[0] = clamp((int32_t)std::floor((localBounds.mn.x() + worldExtent.x() * 0.5f + c_epsilon) / tileWidth), 0, tileCount - 1);
	outRange[1] = clamp((int32_t)std::floor((localBounds.mn.z() + worldExtent.z() * 0.5f + c_epsilon) / tileDepth), 0, tileCount - 1);
	outRange[2] = clamp((int32_t)std::floor((localBounds.mx.x() + worldExtent.x() * 0.5f - c_epsilon) / tileWidth), 0, tileCount - 1);
	outRange[3] = clamp((int32_t)std::floor((localBounds.mx.z() + worldExtent.z() * 0.5f - c_epsilon) / tileDepth), 0, tileCount - 1);
}

//! Stamp of terrain tiles overlapping bounds.
uint32_t readTerrainStamp(editor::IPipelineBuilder* pipelineBuilder, const NavMeshTerrain& terrain, const Aabb3& bounds)
{
	Ref< db::Instance > heightfieldInstance;
	Ref< const hf::HeightfieldAsset > heightfieldAsset;
	if (!readTerrainHeightfield(pipelineBuilder, terrain.terrain, heightfieldInstance, heightfieldAsset))
		return 0;

	int32_t range[4];
	getTerrainTileRange(heightfieldAsset, bounds.transform(terrain.transform.inverse()), range);

	const hf::TileStorage tileStorage(heightfieldAsset->getTileCount());
	return DeepHash(heightfieldAsset).get() + tileStorage.readStamp(heightfieldInstance, L"Data", range[0], range[1], range[2], range[3]);
}

/*! Convert heightfield tiles of terrain overlapping bounds, or every tile if bounds are empty, at full resolution.
 * Tiles include enough samples of their neighbours to cover border around bounds.
 */
bool createTerrainModels(editor::IPipelineBuilder* pipelineBuilder, const NavMeshTerrain& terrain, const Aabb3& bounds, float border, AlignedVector< NavMeshSourceModel >& outNavModels)
{
	Ref< db::Instance > heightfieldInstance;
	Ref< const hf::HeightfieldAsset > heightfieldAsset;
	if (!readTerrainHeightfield(pipelineBuilder, terrain.terrain, heightfieldInstance, heightfieldAsset))
		return false;

	const Vector4& worldExtent = heightfieldAsset->getWorldExtent();
	const hf::TileStorage tileStorage(heightfieldAsset->getTileCount());
	const int32_t tileCount = std::max(heightfieldAsset->getTileCount(), 1);

	int32_t range[4];
	getTerrainTileRange(heightfieldAsset, bounds.empty() ? bounds : bounds.transform(terrain.transform.inverse()), range);

	int32_t borderSamples = 0;
	if (border > 0.0f && heightfieldAsset->getTileCount() > 0)
	{
		const int32_t size = tileStorage.readSize(heightfieldInstance);
		if (size <= 0)
			return false;
		borderSamples = (int32_t)std::ceil(border * size / worldExtent.x()) + 1;
	}

	AlignedVector< Ref< model::Model > > models(tileCount * tileCount);
	AlignedVector< bool > selected(tileCount * tileCount, false);
	RefArray< Job > jobs;
	bool failed = false;

	for (int32_t tz = range[1]; tz <= range[3] && !failed; ++tz)
	{
		for (int32_t tx = range[0]; tx <= range[2] && !failed; ++tx)
		{
			const int32_t tile = tx + tz * tileCount;
			selected[tile] = true;

			Ref< Job > job = JobManager::getInstance().add([&, tx, tz, tile]() {
				Ref< hf::Heightfield > heightfield = (heightfieldAsset->getTileCount() > 0) ?
					tileStorage.readHeightfieldTile(heightfieldInstance, worldExtent, tx, tz, borderSamples) :
					tileStorage.readHeightfield(heightfieldInstance, worldExtent);
				if (heightfield)
					models[tile] = hf::ConvertHeightfield().convert(heightfield, 1, c_heightfieldMaxError);
			});
			if (job)
				jobs.push_back(job);
			else
				failed = true;
		}
	}

	while (!jobs.empty())
	{
		jobs.back()->wait();
		jobs.pop_back();
	}

	if (failed)
		return false;

	for (int32_t i = 0; i < tileCount * tileCount; ++i)
	{
		if (!selected[i])
			continue;
		if (!models[i])
			return false;

		models[i]->apply(model::Triangulate());
		outNavModels.push_back(NavMeshSourceModel(models[i], terrain.transform, Guid()));
	}

	return true;
}

bool buildTile(
	const rcConfig& cfg,
	int32_t tileX,
	int32_t tileY,
	float agentHeight,
	float agentRadius,
	float agentClimb,
	const AlignedVector< float >& vertices,
	const AlignedVector< int32_t >& indices,
	const AlignedVector< uint8_t >& areas,
	const AlignedVector< int32_t >& triangles,
	const AlignedVector< NavMeshModifier >& modifiers,
	model::Model* debugModel,
	Semaphore& debugModelLock,
	AlignedVector< uint8_t >& outTileData,
	int32_t& outPolyCount)
{
	BuildContext ctx;

	outTileData.resize(0);
	outPolyCount = 0;

	if (triangles.empty())
		return true;

	AlignedVector< int32_t > tileIndices;
	AlignedVector< uint8_t > tileAreas;
	tileIndices.reserve(triangles.size() * 3);
	tileAreas.reserve(triangles.size());
	for (const int32_t triangle : triangles)
	{
		tileIndices.push_back(indices[triangle * 3 + 0]);
		tileIndices.push_back(indices[triangle * 3 + 1]);
		tileIndices.push_back(indices[triangle * 3 + 2]);
		tileAreas.push_back(areas[triangle]);
	}

	std::unique_ptr< rcHeightfield, decltype(&rcFreeHeightField) > solid(rcAllocHeightfield(), &rcFreeHeightField);
	if (!solid || !rcCreateHeightfield(&ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
		return false;

	if (!rcRasterizeTriangles(&ctx, vertices.c_ptr(), (int)(vertices.size() / 3), tileIndices.c_ptr(), tileAreas.c_ptr(), (int)tileAreas.size(), *solid, cfg.walkableClimb))
		return false;

	// Remove unwanted overhangs and spans where the character cannot stand.
	rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *solid);
	rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
	rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *solid);

	std::unique_ptr< rcCompactHeightfield, decltype(&rcFreeCompactHeightfield) > chf(rcAllocCompactHeightfield(), &rcFreeCompactHeightfield);
	if (!chf || !rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf))
		return false;

	solid.reset();

	if (!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf))
		return false;

	for (const auto& modifier : modifiers)
	{
		float bmin[3], bmax[3];
		copyUnaligned3(bmin, modifier.bounds.mn);
		copyUnaligned3(bmax, modifier.bounds.mx);
		rcMarkBoxArea(&ctx, bmin, bmax, modifier.area, *chf);
	}

	const bool c_monotonePartitioning = false;
	if (c_monotonePartitioning)
	{
		if (!rcBuildRegionsMonotone(&ctx, *chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))
			return false;
	}
	else
	{
		if (!rcBuildDistanceField(&ctx, *chf))
			return false;
		if (!rcBuildRegions(&ctx, *chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))
			return false;
	}

	std::unique_ptr< rcContourSet, decltype(&rcFreeContourSet) > cset(rcAllocContourSet(), &rcFreeContourSet);
	if (!cset || !rcBuildContours(&ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset))
		return false;

	if (cset->nconts == 0)
		return true;

	std::unique_ptr< rcPolyMesh, decltype(&rcFreePolyMesh) > pmesh(rcAllocPolyMesh(), &rcFreePolyMesh);
	if (!pmesh || !rcBuildPolyMesh(&ctx, *cset, cfg.maxVertsPerPoly, *pmesh))
		return false;

	std::unique_ptr< rcPolyMeshDetail, decltype(&rcFreePolyMeshDetail) > dmesh(rcAllocPolyMeshDetail(), &rcFreePolyMeshDetail);
	if (!dmesh || !rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh))
		return false;

	chf.reset();
	cset.reset();

	if (pmesh->npolys == 0)
		return true;

	for (int i = 0; i < pmesh->npolys; ++i)
		if (pmesh->areas[i] != RC_NULL_AREA)
			pmesh->flags[i] = 0xffff;

	dtNavMeshCreateParams params;
	std::memset(&params, 0, sizeof(params));

	params.verts = pmesh->verts;
	params.vertCount = pmesh->nverts;
	params.polys = pmesh->polys;
	params.polyAreas = pmesh->areas;
	params.polyFlags = pmesh->flags;
	params.polyCount = pmesh->npolys;
	params.nvp = pmesh->nvp;
	params.detailMeshes = dmesh->meshes;
	params.detailVerts = dmesh->verts;
	params.detailVertsCount = dmesh->nverts;
	params.detailTris = dmesh->tris;
	params.detailTriCount = dmesh->ntris;
	params.walkableHeight = agentHeight;
	params.walkableRadius = agentRadius;
	params.walkableClimb = agentClimb;
	params.tileX = tileX;
	params.tileY = tileY;
	rcVcopy(params.bmin, pmesh->bmin);
	rcVcopy(params.bmax, pmesh->bmax);
	params.cs = cfg.cs;
	params.ch = cfg.ch;
	params.buildBvTree = true;

	uint8_t* navData = nullptr;
	int32_t navDataSize = 0;
	if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		return false;

	outTileData.resize(navDataSize);
	std::memcpy(outTileData.ptr(), navData, navDataSize);
	outPolyCount = pmesh->npolys;

	dtFree(navData);

	if (debugModel)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(debugModelLock);

		AlignedVector< uint32_t > vertexIds(pmesh->nverts);
		for (int32_t i = 0; i < pmesh->nverts; ++i)
		{
			const uint32_t positionId = debugModel->addPosition(Vector4(
				pmesh->bmin[0] + pmesh->verts[i * 3 + 0] * pmesh->cs,
				pmesh->bmin[1] + pmesh->verts[i * 3 + 1] * pmesh->ch,
				pmesh->bmin[2] + pmesh->verts[i * 3 + 2] * pmesh->cs,
				1.0f));
			vertexIds[i] = debugModel->addVertex(model::Vertex(positionId));
		}

		for (int32_t i = 0; i < pmesh->npolys; ++i)
		{
			model::Polygon polygon;

			const uint16_t* p = &pmesh->polys[i * pmesh->nvp * 2];
			for (int32_t j = 0; j < pmesh->nvp; ++j)
			{
				if (p[j] == RC_MESH_NULL_IDX)
					break;
				polygon.addVertex(vertexIds[p[j]]);
			}

			polygon.flipWinding();
			debugModel->addPolygon(polygon);
		}
	}

	return true;
}

//! Expand bounds by a cell since marked cells and polygon vertices snap outwards.
void writeModifiers(Writer& w, const rcConfig& cfg, const AlignedVector< NavMeshModifier >& modifiers)
{
	w << int32_t(modifiers.size());
	for (const auto& modifier : modifiers)
	{
		const Aabb3 bounds = modifier.bounds.expand(Scalar(std::max(cfg.cs, cfg.ch)));
		w.write((const uint8_t*)modifier.id, 16);
		w << modifier.area;
		w << bounds.mn.x();
		w << bounds.mn.y();
		w << bounds.mn.z();
		w << bounds.mx.x();
		w << bounds.mx.y();
		w << bounds.mx.z();
	}
}

//! Hash of geometry an entity contribute.
uint32_t hashEntityModel(const world::EntityData* entityData, const model::Model* model)
{
	Murmur3 hash;
	hash.begin();

	const Transform& transform = entityData->getTransform();
	hash.feedBuffer(&transform, sizeof(transform));

	const bool walkable = model->getProperty< bool >(L"Navigation.Walkable", true);
	hash.feedBuffer(&walkable, sizeof(walkable));

	for (const auto& polygon : model->getPolygons())
	{
		for (const auto vertex : polygon.getVertices())
		{
			float position[3];
			copyUnaligned3(position, model->getVertexPosition(vertex));
			hash.feedBuffer(position, sizeof(position));
		}
	}

	hash.end();
	return hash.get();
}

//! True if bounds overlap in X and Z.
bool overlapXZ(const Aabb3& a, const Aabb3& b)
{
	return a.mn.x() < b.mx.x() && a.mx.x() > b.mn.x() && a.mn.z() < b.mx.z() && a.mx.z() > b.mn.z();
}

//! Gather geometry and settings from source scene; only entities in filter are replicated if filter is given.
bool gatherSource(
	editor::IPipelineBuilder* pipelineBuilder,
	const SmallMap< const TypeInfo*, Ref< const world::IEntityReplicator > >& entityReplicators,
	const ISerializable* sourceData,
	const SmallSet< Guid >* entityFilter,
	bool hashEntities,
	NavMeshSource& outSource)
{
	Semaphore lock;
	RefArray< Job > jobs;
	bool failed = false;

	world::Traverser::visit(sourceData, [&](const world::EntityData* entityData) -> world::Traverser::Result {
		// Dynamic layers do not get included in nav.
		if (!entityData->getState().visible || entityData->getState().dynamic)
			return world::Traverser::Result::Skip;

		// Explicitly check for ocean component, need to discard everything below ocean level.
		if (auto oceanComponentData = entityData->getComponent< terrain::OceanComponentData >())
		{
			outSource.oceanHeight = max< float >(outSource.oceanHeight, oceanComponentData->getElevation());
			outSource.oceanClip = true;
		}

		// Check for explicit volume bounds.
		if (auto volumeComponentData = entityData->getComponent< world::VolumeComponentData >())
		{
			if (entityData->getName() == L"NavMesh" && !volumeComponentData->getBoundingBox().empty())
				outSource.maximumBounds = volumeComponentData->getBoundingBox();
		}

		// Terrain is converted from its heightfield tiles once navigation bounds are known.
		if (auto terrainComponentData = entityData->getComponent< terrain::TerrainComponentData >())
		{
			outSource.terrains.push_back({ terrainComponentData->getTerrain(), entityData->getTransform() });
			return world::Traverser::Result::Continue;
		}

		if (entityFilter && entityFilter->find(entityData->getId()) == entityFilter->end())
			return world::Traverser::Result::Continue;

		Ref< Job > job = JobManager::getInstance().add([&, entityData]() {
			Ref< model::Model > model;
			for (auto componentData : entityData->getComponents())
			{
				// Find model synthesizer which can generate from current component.
				const auto it = entityReplicators.find(&type_of(componentData));
				if (it == entityReplicators.end())
					continue;
				if ((model = it->second->createModel(pipelineBuilder, entityData, componentData, world::IEntityReplicator::Usage::Collision, world::IEntityReplicator::Flags::SkipMaterials)) != nullptr)
					break;
			}
			if (!model)
				return;

			model->apply(model::Triangulate());
			const uint32_t hash = hashEntities ? hashEntityModel(entityData, model) : 0;

			T_ANONYMOUS_VAR(Acquire< Semaphore >)(lock);
			outSource.models.push_back(NavMeshSourceModel(model, entityData->getTransform(), entityData->getId()));
			if (hashEntities)
				outSource.entityHashes[entityData->getId()] += hash;
		});
		if (!job)
		{
			failed = true;
			return world::Traverser::Result::Failed;
		}

		jobs.push_back(job);
		return world::Traverser::Result::Continue;
	});

	while (!jobs.empty())
	{
		jobs.back()->wait();
		jobs.pop_back();
	}

	return !failed;
}

//! Modifiers are gathered from dynamic layers as well since their owners are often dynamic.
void collectModifiers(
	editor::IPipelineBuilder* pipelineBuilder,
	const SmallMap< const TypeInfo*, Ref< const world::IEntityReplicator > >& entityReplicators,
	const ISerializable* sourceData,
	float agentClimb,
	AlignedVector< NavMeshModifier >& outModifiers)
{
	AlignedVector< std::pair< Guid, Ref< const world::EntityData > > > modifierEntities;
	collectModifierEntities(sourceData, Guid::null, modifierEntities);

	SmallSet< Guid > modifierIds;
	for (const auto& modifierEntity : modifierEntities)
	{
		const world::EntityData* entityData = modifierEntity.second;
		if (!modifierIds.insert(modifierEntity.first))
		{
			log::warning << L"NavMesh modifier \"" << entityData->getName() << L"\" has same id as another modifier; ignored." << Endl;
			continue;
		}

		// Geometry of owner and its visible children.
		AlignedVector< const world::EntityData* > boundsEntities = { entityData };
		world::Traverser::visit(entityData, [&](const world::EntityData* childEntityData) -> world::Traverser::Result {
			if (!childEntityData->getState().visible)
				return world::Traverser::Result::Skip;
			boundsEntities.push_back(childEntityData);
			return world::Traverser::Result::Continue;
		});

		Aabb3 bounds;
		for (auto boundsEntity : boundsEntities)
		{
			for (auto componentData : boundsEntity->getComponents())
			{
				const auto it = entityReplicators.find(&type_of(componentData));
				if (it == entityReplicators.end())
					continue;

				Ref< model::Model > model = it->second->createModel(pipelineBuilder, boundsEntity, componentData, world::IEntityReplicator::Usage::Collision, world::IEntityReplicator::Flags::SkipMaterials);
				if (!model)
					model = it->second->createModel(pipelineBuilder, boundsEntity, componentData, world::IEntityReplicator::Usage::Visual, world::IEntityReplicator::Flags::SkipMaterials);
				if (model)
					bounds.contain(model->getBoundingBox().transform(boundsEntity->getTransform()));
			}
		}

		if (auto volumeComponentData = entityData->getComponent< world::VolumeComponentData >())
			bounds.contain(volumeComponentData->getBoundingBox().transform(entityData->getTransform()));

		if (bounds.empty())
		{
			log::warning << L"NavMesh modifier \"" << entityData->getName() << L"\" has no bounds; ignored." << Endl;
			continue;
		}

		// Include walkable surface within climb height below modifier.
		bounds.mn -= Vector4(0.0f, agentClimb, 0.0f, 0.0f);

		// Area ids only need to differ between neighbouring modifiers.
		const uint8_t area = (uint8_t)(1 + outModifiers.size() % (RC_WALKABLE_AREA - 1));
		outModifiers.push_back({ modifierEntity.first, bounds, area });
	}
}

//! Build range of tiles in grid; data of tiles without walkable area is empty.
bool buildTiles(
	const rcConfig& cfg,
	float agentHeight,
	float agentRadius,
	float agentClimb,
	AlignedVector< NavMeshSourceModel >& models,
	float oceanHeight,
	bool oceanClip,
	const AlignedVector< NavMeshModifier >& modifiers,
	const NavMeshTileGrid& grid,
	AlignedVector< AlignedVector< uint8_t > >& outTileData,
	AlignedVector< int32_t >& outTilePolyCounts)
{
	// Merge all models into a single world space triangle soup.
	AlignedVector< float > vertices;
	AlignedVector< int32_t > indices;
	AlignedVector< bool > walkables;

	for (auto& navModel : models)
	{
		const int32_t vertexBase = (int32_t)(vertices.size() / 3);
		const uint32_t vertexCount = navModel.model->getVertexCount();
		const uint32_t triangleCount = navModel.model->getPolygonCount();
		const bool walkable = navModel.model->getProperty< bool >(L"Navigation.Walkable", true);

		for (uint32_t j = 0; j < vertexCount; ++j)
		{
			const Vector4 position = navModel.transform * navModel.model->getVertexPosition(j).xyz1();
			vertices.push_back(position.x());
			vertices.push_back(position.y());
			vertices.push_back(position.z());
		}

		for (uint32_t j = 0; j < triangleCount; ++j)
		{
			const model::Polygon& triangle = navModel.model->getPolygon(j);
			T_ASSERT(triangle.getVertexCount() == 3);

			if (oceanClip)
			{
				if (vertices[(vertexBase + triangle.getVertex(0)) * 3 + 1] < oceanHeight - c_oceanThreshold)
					continue;
				if (vertices[(vertexBase + triangle.getVertex(1)) * 3 + 1] < oceanHeight - c_oceanThreshold)
					continue;
				if (vertices[(vertexBase + triangle.getVertex(2)) * 3 + 1] < oceanHeight - c_oceanThreshold)
					continue;
			}

			indices.push_back(vertexBase + triangle.getVertex(2));
			indices.push_back(vertexBase + triangle.getVertex(1));
			indices.push_back(vertexBase + triangle.getVertex(0));
			walkables.push_back(walkable);
		}

		navModel.model = nullptr;
	}

	const int32_t vertexCount = (int32_t)(vertices.size() / 3);
	const int32_t triangleCount = (int32_t)(indices.size() / 3);

	// Mark walkable triangles.
	AlignedVector< uint8_t > areas((size_t)triangleCount, 0);
	if (triangleCount > 0)
	{
		BuildContext ctx;
		rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, vertices.c_ptr(), vertexCount, indices.c_ptr(), triangleCount, areas.ptr());

		// Non-walkable models are kept as obstacles.
		for (int32_t i = 0; i < triangleCount; ++i)
		{
			if (!walkables[i])
				areas[i] = RC_NULL_AREA;
		}
	}

	// Bin triangles into each tile they overlap, including border.
	const float border = cfg.borderSize * cfg.cs;
	AlignedVector< AlignedVector< int32_t > > tileTriangles(grid.tilesX * grid.tilesZ);
	for (int32_t i = 0; i < triangleCount; ++i)
	{
		float mnx = std::numeric_limits< float >::max(), mxx = -std::numeric_limits< float >::max();
		float mnz = std::numeric_limits< float >::max(), mxz = -std::numeric_limits< float >::max();
		for (int32_t j = 0; j < 3; ++j)
		{
			const float* v = &vertices[indices[i * 3 + j] * 3];
			mnx = std::min(mnx, v[0]);
			mxx = std::max(mxx, v[0]);
			mnz = std::min(mnz, v[2]);
			mxz = std::max(mxz, v[2]);
		}

		const int32_t tx0 = std::max((int32_t)std::floor((mnx - border - grid.originX) / grid.tileWidth) - grid.firstX, 0);
		const int32_t tx1 = std::min((int32_t)std::floor((mxx + border - grid.originX) / grid.tileWidth) - grid.firstX, grid.tilesX - 1);
		const int32_t tz0 = std::max((int32_t)std::floor((mnz - border - grid.originZ) / grid.tileDepth) - grid.firstZ, 0);
		const int32_t tz1 = std::min((int32_t)std::floor((mxz + border - grid.originZ) / grid.tileDepth) - grid.firstZ, grid.tilesZ - 1);

		for (int32_t tz = tz0; tz <= tz1; ++tz)
			for (int32_t tx = tx0; tx <= tx1; ++tx)
				tileTriangles[tx + tz * grid.tilesX].push_back(i);
	}

	// Build tiles in parallel.
	Semaphore debugModelLock;
	outTileData.resize(grid.tilesX * grid.tilesZ);
	outTilePolyCounts.resize(grid.tilesX * grid.tilesZ, 0);
	std::atomic< bool > failed = false;
	RefArray< Job > jobs;

	for (int32_t tz = 0; tz < grid.tilesZ && !failed; ++tz)
	{
		for (int32_t tx = 0; tx < grid.tilesX && !failed; ++tx)
		{
			Ref< Job > job = JobManager::getInstance().add([&, tx, tz]() {
				const int32_t tile = tx + tz * grid.tilesX;
				const int32_t gridX = grid.firstX + tx;
				const int32_t gridZ = grid.firstZ + tz;

				rcConfig tileCfg = cfg;
				tileCfg.bmin[0] = grid.originX + gridX * grid.tileWidth - border;
				tileCfg.bmin[1] = grid.minY;
				tileCfg.bmin[2] = grid.originZ + gridZ * grid.tileDepth - border;
				tileCfg.bmax[0] = grid.originX + (gridX + 1) * grid.tileWidth + border;
				tileCfg.bmax[1] = grid.maxY;
				tileCfg.bmax[2] = grid.originZ + (gridZ + 1) * grid.tileDepth + border;

				// Vertical range of tile from its own triangles, within span height limit.
				if (!tileTriangles[tile].empty())
				{
					float mny = std::numeric_limits< float >::max();
					float mxy = -std::numeric_limits< float >::max();
					for (const int32_t triangle : tileTriangles[tile])
					{
						for (int32_t j = 0; j < 3; ++j)
						{
							const float y = vertices[indices[triangle * 3 + j] * 3 + 1];
							mny = std::min(mny, y);
							mxy = std::max(mxy, y);
						}
					}
					tileCfg.bmin[1] = std::max(mny - cfg.ch, grid.minY);
					tileCfg.bmax[1] = std::max(std::min(mxy + agentHeight, grid.maxY), tileCfg.bmin[1] + cfg.ch);
					tileCfg.bmax[1] = std::min(tileCfg.bmax[1], tileCfg.bmin[1] + RC_SPAN_MAX_HEIGHT * cfg.ch);
				}

				if (!buildTile(
					tileCfg,
					gridX,
					gridZ,
					agentHeight,
					agentRadius,
					agentClimb,
					vertices,
					indices,
					areas,
					tileTriangles[tile],
					modifiers,
					nullptr,
					debugModelLock,
					outTileData[tile],
					outTilePolyCounts[tile]))
				{
					log::error << L"NavMesh pipeline failed; unable to build tile " << gridX << L", " << gridZ << L"." << Endl;
					failed = true;
				}
			});
			if (job)
				jobs.push_back(job);
			else
				failed = true;
		}
	}

	while (!jobs.empty())
	{
		jobs.back()->wait();
		jobs.pop_back();
	}

	return !failed;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshPipeline", 23, NavMeshPipeline, editor::DefaultPipeline)

bool NavMeshPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	m_editor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	m_build = settings->getPropertyIncludeHash< bool >(L"NavMeshPipeline.Build", true);

	// Create entity replicators.
	for (const auto& entityReplicatorType : type_of< world::IEntityReplicator >().findAllOf(false))
	{
		Ref< world::IEntityReplicator > entityReplicator = mandatory_non_null_type_cast< world::IEntityReplicator* >(entityReplicatorType->createInstance());
		if (!entityReplicator->create(settings))
			return false;

		auto supportedTypes = entityReplicator->getSupportedTypes();
		for (auto supportedType : supportedTypes)
			m_entityReplicators[supportedType] = entityReplicator;
	}

	return true;
}

TypeInfoSet NavMeshPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< NavMeshAsset, NavMeshChunkAsset >();
}

bool NavMeshPipeline::shouldCache() const
{
	return true;
}

bool NavMeshPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid) const
{
	// Chunk output depend on stamp of what it read rather than on source scene.
	if (is_a< NavMeshChunkAsset >(sourceAsset))
		return true;

	const NavMeshAsset* asset = mandatory_non_null_type_cast< const NavMeshAsset* >(sourceAsset);
	pipelineDepends->addDependency(asset->m_source, editor::PdfUse);

	Ref< const ISerializable > sourceData = pipelineDepends->getObjectReadOnly(asset->m_source);
	if (!sourceData)
		return true;

	AlignedVector< Guid > externalEntities;
	world::resolveExternal(
		[&](const Guid& objectId) -> Ref< const ISerializable > {
		return pipelineDepends->getObjectReadOnly(objectId);
	},
		sourceData,
		Guid::null,
		&externalEntities);

	for (const auto& externalEntity : externalEntities)
		pipelineDepends->addDependency(externalEntity, editor::PdfUse);

	return true;
}

bool NavMeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason) const
{
	if (!m_build)
		return true;

	if (auto chunkAsset = dynamic_type_cast< const NavMeshChunkAsset* >(sourceAsset))
		return buildChunk(pipelineBuilder, chunkAsset, outputPath, outputGuid);

	const NavMeshAsset* asset = mandatory_non_null_type_cast< const NavMeshAsset* >(sourceAsset);
	if (asset->m_chunkSize > 0.0f)
		return buildChunked(pipelineBuilder, sourceInstance, asset, outputPath, outputGuid);
	else
		return buildSingle(pipelineBuilder, sourceInstance, asset, outputPath, outputGuid);
}

bool NavMeshPipeline::calculateChunkGrid(editor::IPipelineBuilder* pipelineBuilder, const NavMeshAsset* asset, ChunkGrid& outGrid)
{
	outGrid = ChunkGrid();
	if (asset->m_chunkSize <= 0.0f)
		return true;

	Ref< const ISerializable > sourceData = buildSourceScene(pipelineBuilder, asset);
	if (!sourceData)
		return false;

	return calculateChunkGrid(pipelineBuilder, asset, sourceData, outGrid);
}

Guid NavMeshPipeline::getChunkId(const Guid& navMeshId, int32_t chunkX, int32_t chunkZ)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = c_guidChunkSeed[i] ^ navMeshId[i];
	d[0] ^= (uint8_t)chunkX;
	d[1] ^= (uint8_t)(chunkX >> 8);
	d[2] ^= (uint8_t)chunkZ;
	d[3] ^= (uint8_t)(chunkZ >> 8);
	return Guid(d);
}

Ref< const ISerializable > NavMeshPipeline::buildSourceScene(editor::IPipelineBuilder* pipelineBuilder, const NavMeshAsset* asset)
{
	Ref< const ISerializable > sourceData = pipelineBuilder->getObjectReadOnly(asset->m_source);
	if (!sourceData)
	{
		log::error << L"NavMesh pipeline failed; unable to read source data." << Endl;
		return nullptr;
	}

	sourceData = pipelineBuilder->buildProduct(nullptr, sourceData);
	if (!sourceData)
	{
		log::error << L"NavMesh pipeline failed; unable to pipeline source data." << Endl;
		return nullptr;
	}

	return world::resolveExternal(
		[&](const Guid& objectId) -> Ref< const ISerializable > {
		return pipelineBuilder->getObjectReadOnly(objectId);
	},
		sourceData,
		Guid::null,
		nullptr);
}

bool NavMeshPipeline::calculateChunkGrid(editor::IPipelineBuilder* pipelineBuilder, const NavMeshAsset* asset, const ISerializable* sourceData, ChunkGrid& outGrid)
{
	outGrid = ChunkGrid();
	if (asset->m_chunkSize <= 0.0f)
		return true;

	if (asset->m_tileSize <= 0)
	{
		log::error << L"NavMesh pipeline failed; chunked navigation mesh must be tiled." << Endl;
		return false;
	}

	// Only terrains and navigation volume decide grid, no entity is replicated.
	const SmallMap< const TypeInfo*, Ref< const world::IEntityReplicator > > noReplicators;
	const SmallSet< Guid > noEntities;
	NavMeshSource source;
	if (!gatherSource(pipelineBuilder, noReplicators, sourceData, &noEntities, false, source))
		return false;

	// Terrains in stable order since first terrain decides origin.
	std::sort(source.terrains.begin(), source.terrains.end(), [](const NavMeshTerrain& lh, const NavMeshTerrain& rh) {
		return lh.terrain < rh.terrain;
	});

	Aabb3 firstTerrainBounds;
	for (const auto& terrain : source.terrains)
	{
		Ref< db::Instance > heightfieldInstance;
		Ref< const hf::HeightfieldAsset > heightfieldAsset;
		if (!readTerrainHeightfield(pipelineBuilder, terrain.terrain, heightfieldInstance, heightfieldAsset))
		{
			log::error << L"NavMesh pipeline failed; unable to read terrain." << Endl;
			return false;
		}

		const Vector4 halfExtent = heightfieldAsset->getWorldExtent() * Scalar(0.5f);
		const Aabb3 terrainBounds = Aabb3((-halfExtent).xyz1(), halfExtent.xyz1()).transform(terrain.transform);
		if (firstTerrainBounds.empty())
			firstTerrainBounds = terrainBounds;
		outGrid.extent.contain(terrainBounds);
	}

	if (!source.maximumBounds.empty())
	{
		if (!outGrid.extent.empty())
		{
			outGrid.extent.mn = max(outGrid.extent.mn, source.maximumBounds.mn);
			outGrid.extent.mx = min(outGrid.extent.mx, source.maximumBounds.mx);
		}
		else
			outGrid.extent = source.maximumBounds;
	}

	if (outGrid.extent.empty())
	{
		log::error << L"NavMesh pipeline failed; chunked navigation mesh needs a terrain or a \"NavMesh\" volume." << Endl;
		return false;
	}

	rcConfig cfg;
	setupConfig(asset, cfg);

	// Chunks are aligned with tiles of first terrain.
	const float tileWidth = asset->m_tileSize * cfg.cs;
	outGrid.origin = firstTerrainBounds.empty() ? outGrid.extent.mn : firstTerrainBounds.mn;
	outGrid.chunkTiles = std::max((int32_t)(asset->m_chunkSize / tileWidth + 0.5f), 1);
	outGrid.chunkWidth = outGrid.chunkTiles * tileWidth;
	outGrid.firstX = (int32_t)std::floor((outGrid.extent.mn.x() - outGrid.origin.x()) / outGrid.chunkWidth);
	outGrid.firstZ = (int32_t)std::floor((outGrid.extent.mn.z() - outGrid.origin.z()) / outGrid.chunkWidth);
	outGrid.lastX = (int32_t)std::ceil((outGrid.extent.mx.x() - outGrid.origin.x()) / outGrid.chunkWidth) - 1;
	outGrid.lastZ = (int32_t)std::ceil((outGrid.extent.mx.z() - outGrid.origin.z()) / outGrid.chunkWidth) - 1;
	return true;
}

bool NavMeshPipeline::buildSingle(editor::IPipelineBuilder* pipelineBuilder, const db::Instance* sourceInstance, const NavMeshAsset* asset, const std::wstring& outputPath, const Guid& outputGuid) const
{
	Ref< const ISerializable > sourceData = buildSourceScene(pipelineBuilder, asset);
	if (!sourceData)
		return false;

	NavMeshSource source;
	if (!gatherSource(pipelineBuilder, m_entityReplicators, sourceData, nullptr, false, source))
		return false;

	AlignedVector< NavMeshModifier > modifiers;
	collectModifiers(pipelineBuilder, m_entityReplicators, sourceData, asset->m_agentClimb, modifiers);

	for (const auto& terrain : source.terrains)
	{
		if (!createTerrainModels(pipelineBuilder, terrain, source.maximumBounds, 0.0f, source.models))
		{
			log::error << L"NavMesh pipeline failed; unable to convert terrain." << Endl;
			return false;
		}
	}

#if 0
	// Create a merged debug mesh.
	Ref< model::Model > debugModel = new model::Model();
	for (const auto& nm : source.models)
	{
		const model::MergeModel mrg(*nm.model, nm.transform, 0.0001f);
		debugModel->apply(mrg);
	}
	model::ModelFormat::writeAny(Path(L"data/Temp/NavMesh_source.obj"), debugModel);
#endif

	// Calculate aabb and count.
	Aabb3 navModelsAabb;
	uint32_t navModelsTriangleCount = 0;

	for (const auto& navModel : source.models)
	{
		navModelsAabb.contain(navModel.model->getBoundingBox().transform(navModel.transform));
		navModelsTriangleCount += navModel.model->getPolygonCount();
	}

	// Override bounds if explicitly given.
	if (!source.maximumBounds.empty())
		navModelsAabb = source.maximumBounds;

	if (navModelsTriangleCount == 0)
	{
		log::error << L"No models for navigation mesh generation found!" << Endl;
		return false;
	}

	log::info << L"\t" << navModelsTriangleCount << L" triangle(s) loaded." << Endl;
	log::info << L"Generating navigation mesh..." << Endl;

	rcConfig cfg;
	setupConfig(asset, cfg);

	copyUnaligned3(cfg.bmin, navModelsAabb.mn);
	copyUnaligned3(cfg.bmax, navModelsAabb.mx);

	int32_t gridWidth = 0, gridHeight = 0;
	rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &gridWidth, &gridHeight);

	// Without tiling the whole grid is built as a single tile.
	const bool tiled = (asset->m_tileSize > 0);
	const int32_t tileCellsX = tiled ? asset->m_tileSize : gridWidth;
	const int32_t tileCellsZ = tiled ? asset->m_tileSize : gridHeight;
	if (!tiled)
	{
		cfg.width = tileCellsX;
		cfg.height = tileCellsZ;
	}

	NavMeshTileGrid grid;
	grid.originX = cfg.bmin[0];
	grid.originZ = cfg.bmin[2];
	grid.tileWidth = tileCellsX * cfg.cs;
	grid.tileDepth = tileCellsZ * cfg.cs;
	grid.minY = cfg.bmin[1];
	grid.maxY = cfg.bmax[1];
	grid.tilesX = (gridWidth + tileCellsX - 1) / tileCellsX;
	grid.tilesZ = (gridHeight + tileCellsZ - 1) / tileCellsZ;

	log::info << L"NavMesh heightfield size " << gridWidth << L" * " << gridHeight << L", " << grid.tilesX << L" * " << grid.tilesZ << L" tile(s)." << Endl;

	AlignedVector< AlignedVector< uint8_t > > tileData;
	AlignedVector< int32_t > tilePolyCounts;
	if (!buildTiles(cfg, asset->m_agentHeight, asset->m_agentRadius, asset->m_agentClimb, source.models, source.oceanHeight, source.oceanClip, modifiers, grid, tileData, tilePolyCounts))
		return false;

	int32_t tileCount = 0;
	int32_t maxTilePolyCount = 0;
	for (int32_t i = 0; i < grid.tilesX * grid.tilesZ; ++i)
	{
		if (!tileData[i].empty())
			++tileCount;
		maxTilePolyCount = std::max(maxTilePolyCount, tilePolyCounts[i]);
	}

	if (tileCount == 0)
	{
		log::error << L"NavMesh pipeline failed; no walkable area found." << Endl;
		return false;
	}

	// Detour packs tile and polygon index into 22 bits of a 32-bit poly reference.
	const int32_t tileBits = (int32_t)dtIlog2(dtNextPow2((unsigned int)(grid.tilesX * grid.tilesZ)));
	const int32_t polyBits = (int32_t)dtIlog2(dtNextPow2((unsigned int)maxTilePolyCount));
	if (tileBits + polyBits > 22)
	{
		log::error << L"NavMesh pipeline failed; " << grid.tilesX * grid.tilesZ << L" tile(s) with up to " << maxTilePolyCount << L" polygon(s) each exceed Detour poly reference bits." << Endl;
		return false;
	}

	// Save navigation data in resource.
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid);
	if (!outputInstance)
	{
		log::error << L"NavMesh pipeline failed; unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(new NavMeshResource());

	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"NavMesh pipeline failed; unable to create data stream." << Endl;
		outputInstance->revert();
		return false;
	}

	Writer w(stream);

	w << uint8_t(4);
	w << cfg.bmin[0];
	w << cfg.bmin[1];
	w << cfg.bmin[2];
	w << grid.tileWidth;
	w << grid.tileDepth;
	w << int32_t(grid.tilesX * grid.tilesZ);
	w << maxTilePolyCount;
	w << tileCount;

	for (const auto& data : tileData)
	{
		if (data.empty())
			continue;

		w << int32_t(data.size());
		if (stream->write(data.c_ptr(), (int64_t)data.size()) != (int64_t)data.size())
		{
			log::error << L"NavMesh pipeline failed; unable to write to data stream." << Endl;
			outputInstance->revert();
			return false;
		}
	}

	writeModifiers(w, cfg, modifiers);

	stream->close();
	stream = nullptr;

	if (!outputInstance->commit())
	{
		log::error << L"NavMesh pipeline failed; unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

bool NavMeshPipeline::buildChunked(editor::IPipelineBuilder* pipelineBuilder, const db::Instance* sourceInstance, const NavMeshAsset* asset, const std::wstring& outputPath, const Guid& outputGuid) const
{
	Ref< const ISerializable > sourceData = buildSourceScene(pipelineBuilder, asset);
	if (!sourceData)
		return false;

	ChunkGrid grid;
	if (!calculateChunkGrid(pipelineBuilder, asset, sourceData, grid))
		return false;

	NavMeshSource source;
	if (!gatherSource(pipelineBuilder, m_entityReplicators, sourceData, nullptr, true, source))
		return false;

	AlignedVector< NavMeshModifier > modifiers;
	collectModifiers(pipelineBuilder, m_entityReplicators, sourceData, asset->m_agentClimb, modifiers);

	rcConfig cfg;
	setupConfig(asset, cfg);

	const float tileWidth = asset->m_tileSize * cfg.cs;
	const int32_t chunkTiles = grid.chunkTiles;
	const float chunkWidth = grid.chunkWidth;
	const float border = (cfg.borderSize + 1) * cfg.cs;
	const Aabb3& extent = grid.extent;
	const Vector4& origin = grid.origin;

	// Terrains in stable order since chunk stamps must not depend on gather order.
	std::sort(source.terrains.begin(), source.terrains.end(), [](const NavMeshTerrain& lh, const NavMeshTerrain& rh) {
		return lh.terrain < rh.terrain;
	});

	AlignedVector< Aabb3 > terrainBounds;
	for (const auto& terrain : source.terrains)
	{
		Ref< db::Instance > heightfieldInstance;
		Ref< const hf::HeightfieldAsset > heightfieldAsset;
		if (!readTerrainHeightfield(pipelineBuilder, terrain.terrain, heightfieldInstance, heightfieldAsset))
		{
			log::error << L"NavMesh pipeline failed; unable to read terrain." << Endl;
			return false;
		}

		const Vector4 halfExtent = heightfieldAsset->getWorldExtent() * Scalar(0.5f);
		terrainBounds.push_back(Aabb3((-halfExtent).xyz1(), halfExtent.xyz1()).transform(terrain.transform));
	}

	const int32_t cx0 = grid.firstX;
	const int32_t cz0 = grid.firstZ;
	const int32_t cx1 = grid.lastX;
	const int32_t cz1 = grid.lastZ;

	log::info << L"NavMesh " << (cx1 - cx0 + 1) << L" * " << (cz1 - cz0 + 1) << L" chunk(s) of " << chunkTiles << L" * " << chunkTiles << L" tile(s)." << Endl;

	Ref< NavMeshAsset > navMesh = DeepClone(asset).create< NavMeshAsset >();
	RefArray< NavMeshChunkAsset > chunks;

	for (int32_t cz = cz0; cz <= cz1; ++cz)
	{
		for (int32_t cx = cx0; cx <= cx1; ++cx)
		{
			const float x0 = origin.x() + cx * chunkWidth;
			const float z0 = origin.z() + cz * chunkWidth;
			const Aabb3 chunkBounds(
				Vector4(x0 - border, extent.mn.y(), z0 - border, 1.0f),
				Vector4(x0 + chunkWidth + border, extent.mx.y(), z0 + chunkWidth + border, 1.0f));

			Ref< NavMeshChunkAsset > chunk = new NavMeshChunkAsset();
			chunk->m_navMesh = navMesh;
			chunk->m_chunkX = cx;
			chunk->m_chunkZ = cz;
			chunk->m_chunkTiles = chunkTiles;
			chunk->m_origin = origin;
			chunk->m_oceanHeight = source.oceanHeight;
			chunk->m_oceanClip = source.oceanClip;

			for (const auto& navModel : source.models)
			{
				if (overlapXZ(navModel.model->getBoundingBox().transform(navModel.transform), chunkBounds))
					chunk->m_entities.push_back(navModel.entityId);
			}

			// Entities in stable order since chunk stamps must not depend on gather order.
			std::sort(chunk->m_entities.begin(), chunk->m_entities.end());
			chunk->m_entities.erase(std::unique(chunk->m_entities.begin(), chunk->m_entities.end()), chunk->m_entities.end());

			uint32_t stamp = 0;
			for (const auto& entityId : chunk->m_entities)
				stamp = stamp * 31 + source.entityHashes[entityId];

			for (size_t i = 0; i < source.terrains.size(); ++i)
			{
				if (!overlapXZ(terrainBounds[i], chunkBounds))
					continue;

				chunk->m_terrains.push_back({ source.terrains[i].terrain, source.terrains[i].transform });
				stamp = stamp * 31 + readTerrainStamp(pipelineBuilder, source.terrains[i], chunkBounds);
			}

			for (const auto& modifier : modifiers)
			{
				if (overlapXZ(modifier.bounds, chunkBounds))
					chunk->m_modifiers.push_back({ modifier.id, modifier.bounds, (int32_t)modifier.area });
			}

			chunk->m_stamp = stamp;
			chunks.push_back(chunk);
		}
	}

	for (const auto chunk : chunks)
	{
		const std::wstring chunkPath = outputPath + L"/Chunks/" + toString(chunk->m_chunkX) + L"_" + toString(chunk->m_chunkZ);
		if (!pipelineBuilder->buildAdHocOutput(chunk, chunkPath, getChunkId(outputGuid, chunk->m_chunkX, chunk->m_chunkZ)))
		{
			log::error << L"NavMesh pipeline failed; unable to build chunk " << chunk->m_chunkX << L", " << chunk->m_chunkZ << L"." << Endl;
			return false;
		}
	}

	// Save chunk grid in resource.
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid);
	if (!outputInstance)
	{
		log::error << L"NavMesh pipeline failed; unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(new NavMeshResource());

	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"NavMesh pipeline failed; unable to create data stream." << Endl;
		outputInstance->revert();
		return false;
	}

	Writer w(stream);

	w << uint8_t(5);
	w << origin.x();
	w << extent.mn.y();
	w << origin.z();
	w << tileWidth;
	w << tileWidth;
	w << chunkTiles;
	w << c_chunkMaxTilePolys;
	w << int32_t(chunks.size());

	for (const auto chunk : chunks)
	{
		const Guid id = getChunkId(outputGuid, chunk->m_chunkX, chunk->m_chunkZ);
		w << chunk->m_chunkX;
		w << chunk->m_chunkZ;
		w.write((const uint8_t*)id, 16);
	}

	writeModifiers(w, cfg, modifiers);

	stream->close();
	stream = nullptr;

	if (!outputInstance->commit())
	{
		log::error << L"NavMesh pipeline failed; unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

bool NavMeshPipeline::buildChunk(editor::IPipelineBuilder* pipelineBuilder, const NavMeshChunkAsset* chunkAsset, const std::wstring& outputPath, const Guid& outputGuid) const
{
	const NavMeshAsset* asset = chunkAsset->m_navMesh;
	if (!asset)
		return false;

	Ref< const ISerializable > sourceData = buildSourceScene(pipelineBuilder, asset);
	if (!sourceData)
		return false;

	SmallSet< Guid > entityFilter;
	for (const auto& entityId : chunkAsset->m_entities)
		entityFilter.insert(entityId);

	NavMeshSource source;
	if (!gatherSource(pipelineBuilder, m_entityReplicators, sourceData, &entityFilter, false, source))
		return false;

	rcConfig cfg;
	setupConfig(asset, cfg);

	NavMeshTileGrid grid;
	grid.originX = chunkAsset->m_origin.x();
	grid.originZ = chunkAsset->m_origin.z();
	grid.tileWidth = asset->m_tileSize * cfg.cs;
	grid.tileDepth = grid.tileWidth;
	grid.minY = -std::numeric_limits< float >::max();
	grid.maxY = std::numeric_limits< float >::max();
	grid.firstX = chunkAsset->m_chunkX * chunkAsset->m_chunkTiles;
	grid.firstZ = chunkAsset->m_chunkZ * chunkAsset->m_chunkTiles;
	grid.tilesX = chunkAsset->m_chunkTiles;
	grid.tilesZ = chunkAsset->m_chunkTiles;

	const float x0 = grid.originX + grid.firstX * grid.tileWidth;
	const float z0 = grid.originZ + grid.firstZ * grid.tileDepth;
	const Aabb3 chunkBounds(
		Vector4(x0, -1.0f, z0, 1.0f),
		Vector4(x0 + grid.tilesX * grid.tileWidth, 1.0f, z0 + grid.tilesZ * grid.tileDepth, 1.0f));
	const float border = (cfg.borderSize + 1) * cfg.cs;

	for (const auto& terrain : chunkAsset->m_terrains)
	{
		if (!createTerrainModels(pipelineBuilder, { terrain.terrain, terrain.transform }, chunkBounds, border, source.models))
		{
			log::error << L"NavMesh pipeline failed; unable to convert terrain." << Endl;
			return false;
		}
	}

	AlignedVector< NavMeshModifier > modifiers;
	for (const auto& modifier : chunkAsset->m_modifiers)
		modifiers.push_back({ modifier.id, modifier.bounds, (uint8_t)modifier.area });

	AlignedVector< AlignedVector< uint8_t > > tileData;
	AlignedVector< int32_t > tilePolyCounts;
	if (!buildTiles(cfg, asset->m_agentHeight, asset->m_agentRadius, asset->m_agentClimb, source.models, chunkAsset->m_oceanHeight, chunkAsset->m_oceanClip, modifiers, grid, tileData, tilePolyCounts))
		return false;

	int32_t tileCount = 0;
	for (int32_t i = 0; i < grid.tilesX * grid.tilesZ; ++i)
	{
		if (tilePolyCounts[i] > c_chunkMaxTilePolys)
		{
			log::error << L"NavMesh pipeline failed; tile " << grid.firstX + i % grid.tilesX << L", " << grid.firstZ + i / grid.tilesX << L" has " << tilePolyCounts[i] << L" polygon(s), more than " << c_chunkMaxTilePolys << L"." << Endl;
			return false;
		}
		if (!tileData[i].empty())
			++tileCount;
	}

	// Save tiles of chunk in resource.
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid);
	if (!outputInstance)
	{
		log::error << L"NavMesh pipeline failed; unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(new NavMeshResource());

	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"NavMesh pipeline failed; unable to create data stream." << Endl;
		outputInstance->revert();
		return false;
	}

	Writer w(stream);

	w << uint8_t(1);
	w << tileCount;

	for (const auto& data : tileData)
	{
		if (data.empty())
			continue;

		w << int32_t(data.size());
		if (stream->write(data.c_ptr(), (int64_t)data.size()) != (int64_t)data.size())
		{
			log::error << L"NavMesh pipeline failed; unable to write to data stream." << Endl;
			outputInstance->revert();
			return false;
		}
	}

	stream->close();
	stream = nullptr;

	if (!outputInstance->commit())
	{
		log::error << L"NavMesh pipeline failed; unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

void NavMeshPipeline::setupConfig(const NavMeshAsset* asset, rcConfig& outCfg)
{
	std::memset(&outCfg, 0, sizeof(outCfg));
	outCfg.cs = asset->m_cellSize;
	outCfg.ch = asset->m_cellHeight;
	outCfg.walkableSlopeAngle = asset->m_agentSlope;
	outCfg.walkableHeight = int(std::ceil(asset->m_agentHeight / outCfg.ch));
	outCfg.walkableClimb = int(std::floor(asset->m_agentClimb / outCfg.ch));
	outCfg.walkableRadius = int(std::ceil(asset->m_agentRadius / outCfg.cs));
	outCfg.maxEdgeLen = int(asset->m_maxEdgeLength / asset->m_cellSize);
	outCfg.maxSimplificationError = asset->m_maxSimplificationError;
	outCfg.minRegionArea = int(asset->m_minRegionSize * asset->m_minRegionSize);
	outCfg.mergeRegionArea = int(asset->m_mergeRegionSize * asset->m_mergeRegionSize);
	outCfg.maxVertsPerPoly = 6;
	outCfg.detailSampleDist = (asset->m_detailSampleDistance < 0.9f) ? std::numeric_limits< float >::max() : asset->m_cellSize * asset->m_detailSampleDistance;
	outCfg.detailSampleMaxError = asset->m_cellHeight * asset->m_detailSampleMaxError;
	outCfg.tileSize = asset->m_tileSize;
	outCfg.borderSize = (asset->m_tileSize > 0) ? outCfg.walkableRadius + 3 : 0;
	outCfg.width = asset->m_tileSize + outCfg.borderSize * 2;
	outCfg.height = asset->m_tileSize + outCfg.borderSize * 2;
}

}
