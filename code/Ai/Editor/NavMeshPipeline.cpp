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
#include "Ai/NavMeshResource.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/JobManager.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/Triangulate.h"
#include "Terrain/OceanComponentData.h"
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

	NavMeshSourceModel() = default;

	NavMeshSourceModel(const model::Model* model_, const Transform& transform_)
		: model(model_)
		, transform(transform_)
	{
	}
};

void copyUnaligned3(float out[3], const Vector4& source)
{
	out[0] = source.x();
	out[1] = source.y();
	out[2] = source.z();
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
		if (pmesh->areas[i] == RC_WALKABLE_AREA)
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

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshPipeline", 14, NavMeshPipeline, editor::DefaultPipeline)

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
	return makeTypeInfoSet< NavMeshAsset >();
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
	const NavMeshAsset* asset = mandatory_non_null_type_cast< const NavMeshAsset* >(sourceAsset);
	pipelineDepends->addDependency(asset->m_source, editor::PdfUse);

	// The navigation mesh is generated from the geometry of the source scene, so every
	// external entity the scene reference is an input to this build and not merely
	// something the scene happen to point at. The scene itself declare them PdfBuild
	// only, which is right for the scene -- it keeps the reference and lets the external
	// build into an instance of its own -- but the global hash deciding whether to
	// rebuild only recurse through PdfUse edges, so without adding them here again the
	// nav mesh is never rebuilt when an external entity is modified.
	//
	// Resolved the same way buildOutput does, so the two agree on what was consumed.
	// Note this walks the scene as authored; entities introduced by scene operators are
	// covered through the scene's own hash rather than from here.
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
	const NavMeshAsset* asset = mandatory_non_null_type_cast< const NavMeshAsset* >(sourceAsset);

	if (!m_build)
		return true;

	Ref< const ISerializable > sourceData = pipelineBuilder->getObjectReadOnly(asset->m_source);
	if (!sourceData)
	{
		log::error << L"NavMesh pipeline failed; unable to read source data." << Endl;
		return false;
	}

	sourceData = pipelineBuilder->buildProduct(sourceInstance, sourceData);
	if (!sourceData)
	{
		log::error << L"NavMesh pipeline failed; unable to pipeline source data." << Endl;
		return false;
	}

	sourceData = world::resolveExternal(
		[&](const Guid& objectId) -> Ref< const ISerializable > {
		return pipelineBuilder->getObjectReadOnly(objectId);
	},
		sourceData,
		Guid::null,
		nullptr);

	AlignedVector< NavMeshSourceModel > navModels;
	Semaphore navModelsLock;
	Aabb3 navMaximumBounds;
	float oceanHeight = -std::numeric_limits< float >::max();
	bool oceanClip = false;
	RefArray< Job > jobs;

	world::Traverser::visit(sourceData, [&](const world::EntityData* entityData) -> world::Traverser::Result {
		// Dynamic layers do not get included in nav.
		if (!entityData->getState().visible || entityData->getState().dynamic)
			return world::Traverser::Result::Skip;

		Ref< Job > job = JobManager::getInstance().add([=, this, &navModels, &navModelsLock, &navMaximumBounds, &oceanHeight, &oceanClip]() {
			Ref< model::Model > model;
			for (auto componentData : entityData->getComponents())
			{
				// Find model synthesizer which can generate from current component.
				const world::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
				if (entityReplicator)
				{
					if ((model = entityReplicator->createModel(pipelineBuilder, entityData, componentData, world::IEntityReplicator::Usage::Collision, world::IEntityReplicator::Flags::SkipMaterials)) != nullptr)
						break;
				}
			}

			// Explicitly check for ocean component, need to discard everything below ocean level.
			if (auto oceanComponentData = entityData->getComponent< terrain::OceanComponentData >())
			{
				oceanHeight = max< float >(oceanHeight, oceanComponentData->getElevation());
				oceanClip = true;
			}

			// Check for explicit volume bounds.
			if (auto volumeComponentData = entityData->getComponent< world::VolumeComponentData >())
			{
				if (entityData->getName() == L"NavMesh" && !volumeComponentData->getBoundingBox().empty())
					navMaximumBounds = volumeComponentData->getBoundingBox();
			}

			if (model)
			{
				model->apply(model::Triangulate());
				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(navModelsLock);
					navModels.push_back(NavMeshSourceModel(
						model,
						entityData->getTransform()));
				}
			}
		});
		if (!job)
			return world::Traverser::Result::Failed;

		jobs.push_back(job);
		return world::Traverser::Result::Continue;
	});

	while (!jobs.empty())
	{
		jobs.back()->wait();
		jobs.pop_back();
	}

#if 1
	// Create a merged debug mesh.
	Ref< model::Model > debugModel = new model::Model();
	for (const auto& nm : navModels)
	{
		const model::MergeModel mrg(*nm.model, nm.transform, 0.0001f);
		debugModel->apply(mrg);
	}
	model::ModelFormat::writeAny(Path(L"data/Temp/NavMesh_source.obj"), debugModel);
#endif

	// Calculate aabb and count.
	Aabb3 navModelsAabb;
	uint32_t navModelsTriangleCount = 0;

	for (uint32_t i = 0; i < navModels.size(); ++i)
	{
		const model::Model* navModel = navModels[i].model;
		T_ASSERT(navModel);

		navModelsAabb.contain(navModel->getBoundingBox().transform(navModels[i].transform));
		navModelsTriangleCount += navModel->getPolygonCount();
	}

	// Override bounds if explicitly given.
	if (!navMaximumBounds.empty())
		navModelsAabb = navMaximumBounds;

	if (navModelsTriangleCount == 0)
	{
		log::error << L"No models for navigation mesh generation found!" << Endl;
		return false;
	}

	log::info << L"\t" << navModelsTriangleCount << L" triangle(s) loaded." << Endl;
	log::info << L"Generating navigation mesh..." << Endl;

	// Merge all models into a single world space triangle soup.
	AlignedVector< float > vertices;
	AlignedVector< int32_t > indices;
	indices.reserve(navModelsTriangleCount * 3);

	for (auto& navModel : navModels)
	{
		const int32_t vertexBase = (int32_t)(vertices.size() / 3);
		const uint32_t vertexCount = navModel.model->getVertexCount();
		const uint32_t triangleCount = navModel.model->getPolygonCount();

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
		}

		navModel.model = nullptr;
	}

	const int32_t vertexCount = (int32_t)(vertices.size() / 3);
	const int32_t triangleCount = (int32_t)(indices.size() / 3);

	rcConfig cfg;
	std::memset(&cfg, 0, sizeof(cfg));
	cfg.cs = asset->m_cellSize;
	cfg.ch = asset->m_cellHeight;
	cfg.walkableSlopeAngle = asset->m_agentSlope;
	cfg.walkableHeight = int(std::ceil(asset->m_agentHeight / cfg.ch));
	cfg.walkableClimb = int(std::floor(asset->m_agentClimb / cfg.ch));
	cfg.walkableRadius = int(std::ceil(asset->m_agentRadius / cfg.cs));
	cfg.maxEdgeLen = int(asset->m_maxEdgeLength / asset->m_cellSize);
	cfg.maxSimplificationError = asset->m_maxSimplificationError;
	cfg.minRegionArea = int(asset->m_minRegionSize * asset->m_minRegionSize);
	cfg.mergeRegionArea = int(asset->m_mergeRegionSize * asset->m_mergeRegionSize);
	cfg.maxVertsPerPoly = 6;
	cfg.detailSampleDist = (asset->m_detailSampleDistance < 0.9f) ? 0.0f : asset->m_cellSize * asset->m_detailSampleDistance;
	cfg.detailSampleMaxError = asset->m_cellHeight * asset->m_detailSampleMaxError;

	copyUnaligned3(cfg.bmin, navModelsAabb.mn);
	copyUnaligned3(cfg.bmax, navModelsAabb.mx);

	int32_t gridWidth = 0, gridHeight = 0;
	rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &gridWidth, &gridHeight);

	// Without tiling the whole grid is built as a single tile.
	const bool tiled = (asset->m_tileSize > 0);
	const int32_t tileCellsX = tiled ? asset->m_tileSize : gridWidth;
	const int32_t tileCellsZ = tiled ? asset->m_tileSize : gridHeight;
	const int32_t tilesX = (gridWidth + tileCellsX - 1) / tileCellsX;
	const int32_t tilesZ = (gridHeight + tileCellsZ - 1) / tileCellsZ;
	const float tileWidth = tileCellsX * cfg.cs;
	const float tileDepth = tileCellsZ * cfg.cs;

	cfg.tileSize = tiled ? asset->m_tileSize : 0;
	cfg.borderSize = tiled ? cfg.walkableRadius + 3 : 0;
	cfg.width = tileCellsX + cfg.borderSize * 2;
	cfg.height = tileCellsZ + cfg.borderSize * 2;

	const float border = cfg.borderSize * cfg.cs;

	log::info << L"NavMesh heightfield size " << gridWidth << L" * " << gridHeight << L", " << tilesX << L" * " << tilesZ << L" tile(s)." << Endl;

	// Mark walkable triangles.
	AlignedVector< uint8_t > areas((size_t)triangleCount, 0);
	if (triangleCount > 0)
	{
		BuildContext ctx;
		rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, vertices.c_ptr(), vertexCount, indices.c_ptr(), triangleCount, areas.ptr());
	}

	// Bin triangles into each tile they overlap, including border.
	AlignedVector< AlignedVector< int32_t > > tileTriangles(tilesX * tilesZ);
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

		const int32_t tx0 = std::max((int32_t)std::floor((mnx - border - cfg.bmin[0]) / tileWidth), 0);
		const int32_t tx1 = std::min((int32_t)std::floor((mxx + border - cfg.bmin[0]) / tileWidth), tilesX - 1);
		const int32_t tz0 = std::max((int32_t)std::floor((mnz - border - cfg.bmin[2]) / tileDepth), 0);
		const int32_t tz1 = std::min((int32_t)std::floor((mxz + border - cfg.bmin[2]) / tileDepth), tilesZ - 1);

		for (int32_t tz = tz0; tz <= tz1; ++tz)
			for (int32_t tx = tx0; tx <= tx1; ++tx)
				tileTriangles[tx + tz * tilesX].push_back(i);
	}

	// Build tiles in parallel.
	Ref< model::Model > navDebugModel = m_editor ? new model::Model() : nullptr;
	Semaphore navDebugModelLock;
	AlignedVector< AlignedVector< uint8_t > > tileData(tilesX * tilesZ);
	AlignedVector< int32_t > tilePolyCounts((size_t)(tilesX * tilesZ), 0);
	std::atomic< bool > failed = false;

	for (int32_t tz = 0; tz < tilesZ; ++tz)
	{
		for (int32_t tx = 0; tx < tilesX; ++tx)
		{
			Ref< Job > job = JobManager::getInstance().add([&, tx, tz]() {
				const int32_t tile = tx + tz * tilesX;

				rcConfig tileCfg = cfg;
				tileCfg.bmin[0] = cfg.bmin[0] + tx * tileWidth - border;
				tileCfg.bmin[2] = cfg.bmin[2] + tz * tileDepth - border;
				tileCfg.bmax[0] = cfg.bmin[0] + (tx + 1) * tileWidth + border;
				tileCfg.bmax[2] = cfg.bmin[2] + (tz + 1) * tileDepth + border;

				if (!buildTile(
					tileCfg,
					tx,
					tz,
					asset->m_agentHeight,
					asset->m_agentRadius,
					asset->m_agentClimb,
					vertices,
					indices,
					areas,
					tileTriangles[tile],
					navDebugModel,
					navDebugModelLock,
					tileData[tile],
					tilePolyCounts[tile]))
				{
					log::error << L"NavMesh pipeline failed; unable to build tile " << tx << L", " << tz << L"." << Endl;
					failed = true;
				}
			});
			if (!job)
				return false;

			jobs.push_back(job);
		}
	}

	while (!jobs.empty())
	{
		jobs.back()->wait();
		jobs.pop_back();
	}

	if (failed)
		return false;

	int32_t tileCount = 0;
	int32_t maxTilePolyCount = 0;
	for (int32_t i = 0; i < tilesX * tilesZ; ++i)
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
	const int32_t tileBits = (int32_t)dtIlog2(dtNextPow2((unsigned int)(tilesX * tilesZ)));
	const int32_t polyBits = (int32_t)dtIlog2(dtNextPow2((unsigned int)maxTilePolyCount));
	if (tileBits + polyBits > 22)
	{
		log::error << L"NavMesh pipeline failed; " << tilesX * tilesZ << L" tile(s) with up to " << maxTilePolyCount << L" polygon(s) each exceed Detour poly reference bits." << Endl;
		return false;
	}

	// Save navigation data in resource.
	Ref< NavMeshResource > outputResource = new NavMeshResource();

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid);
	if (!outputInstance)
	{
		log::error << L"NavMesh pipeline failed; unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(outputResource);

	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"NavMesh pipeline failed; unable to create data stream." << Endl;
		outputInstance->revert();
		return false;
	}

	Writer w(stream);

	w << uint8_t(3);
	w << cfg.bmin[0];
	w << cfg.bmin[1];
	w << cfg.bmin[2];
	w << tileWidth;
	w << tileDepth;
	w << int32_t(tilesX * tilesZ);
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

	stream->close();
	stream = nullptr;

	if (!outputInstance->commit())
	{
		log::error << L"NavMesh pipeline failed; unable to commit output instance." << Endl;
		return false;
	}

	// Save navigation mesh for debugging; only in editor.
	if (navDebugModel)
	{
		navDebugModel->apply(model::Triangulate());
		model::ModelFormat::writeAny(L"data/Temp/NavMesh_nav.obj", navDebugModel);
	}

	return true;
}

}
