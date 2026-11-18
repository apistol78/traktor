/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/TerrainComponent.h"

#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Heightfield/Heightfield.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/IRenderSystem.h"
#include "Render/ITexture.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/TerrainUtilities.h"
#include "World/IWorldRenderPass.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace traktor::terrain
{
namespace
{

const render::Handle c_handleTerrain_VisualizeLods(L"Terrain_VisualizeLods");
const render::Handle c_handleTerrain_VisualizeMap(L"Terrain_VisualizeMap");
const render::Handle c_handleTerrain_SurfaceAlbedo(L"Terrain_SurfaceAlbedo");
const render::Handle c_handleTerrain_Heightfield(L"Terrain_Heightfield");
const render::Handle c_handleTerrain_ColorMap(L"Terrain_ColorMap");
const render::Handle c_handleTerrain_SplatMap(L"Terrain_SplatMap");
const render::Handle c_handleTerrain_CutMap(L"Terrain_CutMap");
const render::Handle c_handleTerrain_Normals(L"Terrain_Normals");
const render::Handle c_handleTerrain_Eye(L"Terrain_Eye");
const render::Handle c_handleTerrain_WorldOrigin(L"Terrain_WorldOrigin");
const render::Handle c_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle c_handleTerrain_PatchExtent(L"Terrain_PatchExtent");
const render::Handle c_handleTerrain_TileTransform(L"Terrain_TileTransform");
const render::Handle c_handleTerrain_DebugMap(L"Terrain_DebugMap");
const render::Handle c_handleTerrain_DebugPatchIndex(L"Terrain_DebugPatchIndex");
const render::Handle c_handleTerrain_CutEnable(L"Terrain_CutEnable");
const render::Handle c_handleTerrain_PatchData(L"Terrain_PatchData");

const uint32_t c_rtPatchCells = 64;	//!< Number of cells along each side of a ray tracing patch.
const float c_rtCellSize = 2.0f;	//!< Approximate size of a ray tracing cell, in world units.

const int32_t c_patchLodSteps = 3;

const float c_tileLoadDistance = 640.0f;	//!< Tiles within this distance from camera are loaded.
const float c_tileUnloadDistance = 768.0f;	//!< Tiles beyond this distance from camera are unloaded.

struct DrawData
{
	float patchOrigin[4];
	float surfaceOffset[4];
	float patchBoundingBoxMn[4];
	float patchBoundingBoxMx[4];
};

Ref< render::ITexture > create1x1Texture(render::IRenderSystem* renderSystem, const Color4ub& value)
{
	render::SimpleTextureCreateDesc stcd = {};
	stcd.width = 1;
	stcd.height = 1;
	stcd.mipCount = 1;
	stcd.format = render::TfR8G8B8A8;
	stcd.sRGB = false;
	stcd.immutable = true;
	stcd.initialData[0].data = &value;
	stcd.initialData[0].pitch = 4;
	return renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
}

//! Horizontal distance from position to tile's area.
float distanceToTile(const Vector4& position, const Aabb3& boundingBox)
{
	const float dx = std::max(std::max((float)(boundingBox.mn.x() - position.x()), (float)(position.x() - boundingBox.mx.x())), 0.0f);
	const float dz = std::max(std::max((float)(boundingBox.mn.z() - position.z()), (float)(position.z() - boundingBox.mx.z())), 0.0f);
	return std::sqrt(dx * dx + dz * dz);
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainComponent", TerrainComponent, world::IEntityComponent)

TerrainComponent::TerrainComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
	: m_resourceManager(resourceManager)
	, m_renderSystem(renderSystem)
{
}

bool TerrainComponent::create(const TerrainComponentData& data)
{
	if (!m_resourceManager->bind(data.getTerrain(), m_terrain))
		return false;

	if (!createGeometry())
		return false;
	if (!createTiles())
		return false;

	// Ray tracing patches are created as surface cache is created.
	m_terrain.consume();

	m_defaultColorMap = create1x1Texture(m_renderSystem, Color4ub(128, 128, 128, 128));
	m_defaultCutMap = create1x1Texture(m_renderSystem, Color4ub(0, 0, 0, 0));
	return true;
}

void TerrainComponent::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	float detailDistance,
	uint32_t cacheSize)
{
	const int32_t viewIndex = worldRenderView.getIndex();
	const bool snapshot = worldRenderView.getSnapshot();

	if (!validate(viewIndex, cacheSize))
		return;

	// Tiles are streamed around primary view, ray tracing patches follow tiles.
	if (viewIndex == 0 && !snapshot)
	{
		updateStreaming(worldRenderView.getEyePosition());
		updateRayTracingPatches();
	}

	// Calculate world frustum.
	const Matrix44 viewInv = worldRenderView.getView().inverse();
	Frustum worldCullFrustum = worldRenderView.getCullFrustum();
	for (uint32_t i = 0; i < worldCullFrustum.planes.size(); ++i)
		worldCullFrustum.planes[i] = viewInv * worldCullFrustum.planes[i];

	for (auto& tile : m_tiles)
		if (tile.loaded)
			setupTile(tile, viewIndex, worldRenderView, worldCullFrustum, snapshot);
	setupTile(m_overview, viewIndex, worldRenderView, worldCullFrustum, snapshot);

	// Update base color texture.
	if (!snapshot)
	{
		const Vector4& worldExtent = m_overview.worldExtent;
		m_surfaceCache->setupBaseColor(
			context.getRenderGraph(),
			m_terrain,
			-worldExtent * 0.5_simd,
			worldExtent);
	}
}

void TerrainComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	float detailDistance,
	uint32_t cacheSize)
{
	const int32_t viewIndex = worldRenderView.getIndex();

	if (!validate(viewIndex, cacheSize))
		return;

	for (const auto& tile : m_tiles)
		if (tile.loaded)
			buildTile(tile, viewIndex, context, worldRenderView, worldRenderPass);
	buildTile(m_overview, viewIndex, context, worldRenderView, worldRenderPass);
}

void TerrainComponent::setVisualizeMode(VisualizeMode visualizeMode)
{
	m_visualizeMode = visualizeMode;
}

bool TerrainComponent::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const
{
	bool found = false;
	outDistance = Scalar(std::numeric_limits< float >::max());

	for (const auto& tile : m_tiles)
	{
		const hf::Heightfield* heightfield = tile.resources.heightfield;
		Scalar distance;
		if (tile.loaded && heightfield && heightfield->queryRay(worldRayOrigin, worldRayDirection, distance) && distance < outDistance)
		{
			outDistance = distance;
			found = true;
		}
	}

	// Overview is only valid where tile isn't loaded.
	const hf::Heightfield* overviewHeightfield = m_overview.resources.heightfield;
	Scalar distance;
	if (overviewHeightfield && overviewHeightfield->queryRay(worldRayOrigin, worldRayDirection, distance) && distance < outDistance)
	{
		const Vector4 position = worldRayOrigin + worldRayDirection * distance;
		for (const auto& tile : m_tiles)
		{
			if (
				!tile.loaded &&
				position.x() >= tile.boundingBox.mn.x() && position.x() <= tile.boundingBox.mx.x() &&
				position.z() >= tile.boundingBox.mn.z() && position.z() <= tile.boundingBox.mx.z()
			)
			{
				outDistance = distance;
				found = true;
				break;
			}
		}
	}

	return found;
}

float TerrainComponent::getWorldHeight(float worldX, float worldZ) const
{
	const hf::Heightfield* heightfield = getHeightfield(worldX, worldZ);
	return heightfield ? heightfield->getWorldHeight(worldX, worldZ) : 0.0f;
}

const hf::Heightfield* TerrainComponent::getTileHeightfield(int32_t tileIndex) const
{
	const Tile& tile = m_tiles[tileIndex];
	return tile.loaded ? tile.resources.heightfield.getResource() : nullptr;
}

void TerrainComponent::destroy()
{
	if (m_loadJob)
	{
		m_loadJob->wait();
		m_loadJob = nullptr;
		m_loadingTile = -1;
	}

	for (auto& tile : m_tiles)
	{
		safeDestroy(tile.drawBuffer);
		safeDestroy(tile.dataBuffer);
	}
	m_tiles.clear();

	safeDestroy(m_overview.drawBuffer);
	safeDestroy(m_overview.dataBuffer);

	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_rtwInstance);
}

void TerrainComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void TerrainComponent::setWorld(world::World* world)
{
	// Remove from last world.
	safeDestroy(m_rtwInstance);

	// Add to new world.
	if (world != nullptr)
	{
		T_FATAL_ASSERT(m_rtwInstance == nullptr);
		world::RTWorldComponent* rtw = world->getComponent< world::RTWorldComponent >();
		if (rtw != nullptr)
			m_rtwInstance = rtw->createInstance(m_rtParts, false);
	}

	m_world = world;
}

void TerrainComponent::setTransform(const Transform& transform)
{
}

Aabb3 TerrainComponent::getBoundingBox() const
{
	const Vector4& worldExtent = m_overview.worldExtent;
	return Aabb3(-worldExtent, worldExtent);
}

void TerrainComponent::update(const world::UpdateParams& update)
{
}

bool TerrainComponent::validate(int32_t viewIndex, uint32_t cacheSize)
{
	const bool cacheSizeChange = (m_surfaceCache == nullptr) || (cacheSize != m_cacheSize);
	if (cacheSizeChange)
	{
		safeDestroy(m_surfaceCache);

		m_surfaceCache = new TerrainSurfaceCache();
		if (!m_surfaceCache->create(m_resourceManager, m_renderSystem, cacheSize))
		{
			m_surfaceCache = nullptr;
			return false;
		}

		m_cacheSize = cacheSize;
	}

	if (m_terrain.changed())
	{
		m_terrain.consume();

		if (!createGeometry())
			return false;
		if (!createTiles())
			return false;
	}
	else if (!cacheSizeChange)
		return true;

	// Ray tracing patches reference base color of surface cache.
	return createRayTracingPatches();
}

bool TerrainComponent::createGeometry()
{
	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);

	const uint32_t patchDim = m_terrain->getPatchDim();
	const uint32_t patchVertexCount = patchDim * patchDim;

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, 0));
	const uint32_t vertexSize = render::getVertexSize(vertexElements);

	m_vertexBuffer = m_renderSystem->createBuffer(
		render::BuVertex,
		patchVertexCount * vertexSize,
		false,
		T_FILE_LINE_W);
	if (!m_vertexBuffer)
		return false;

	float* vertex = static_cast< float* >(m_vertexBuffer->lock());
	T_ASSERT_M(vertex, L"Unable to lock vertex buffer");

	for (uint32_t z = 0; z < patchDim; ++z)
	{
		for (uint32_t x = 0; x < patchDim; ++x)
		{
			*vertex++ = float(x) / (patchDim - 1);
			*vertex++ = float(z) / (patchDim - 1);
		}
	}

	m_vertexBuffer->unlock();

	m_vertexLayout = m_renderSystem->createVertexLayout(vertexElements);

	AlignedVector< uint32_t > indices;
	for (uint32_t lod = 0; lod < LodCount; ++lod)
	{
		const size_t indexOffset = indices.size();
		const uint32_t lodSkip = 1 << lod;

		for (uint32_t y = 0; y < patchDim - 1; y += lodSkip)
		{
			const uint32_t offset = y * patchDim;
			for (uint32_t x = 0; x < patchDim - 1; x += lodSkip)
			{
				if (lod > 0 && (x == 0 || y == 0 || x == patchDim - 1 - lodSkip || y == patchDim - 1 - lodSkip))
				{
					const int mid = x + offset + (lodSkip >> 1) + (lodSkip >> 1) * patchDim;

					if (x == 0)
					{
						indices.push_back(mid);
						indices.push_back(lodSkip + offset);
						indices.push_back(lodSkip + offset + lodSkip * patchDim);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(offset + i * patchDim + patchDim);
							indices.push_back(offset + i * patchDim);
						}
					}
					else if (x == patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim);
						indices.push_back(x + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i * patchDim + lodSkip);
							indices.push_back(x + offset + i * patchDim + lodSkip + patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim);
						indices.push_back(x + offset);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip);
						indices.push_back(x + offset + lodSkip + lodSkip * patchDim);
					}

					if (y == 0)
					{
						indices.push_back(mid);
						indices.push_back(x + lodSkip * patchDim + offset + lodSkip);
						indices.push_back(x + lodSkip * patchDim + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i);
							indices.push_back(x + offset + i + 1);
						}
					}
					else if (y == patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i + lodSkip * patchDim + 1);
							indices.push_back(x + offset + i + lodSkip * patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim + lodSkip);
						indices.push_back(x + offset + lodSkip * patchDim);
					}
				}
				else
				{
					indices.push_back(x + offset);
					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + x + offset);

					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + x + offset);
				}
			}
		}

		const size_t indexEndOffset = indices.size();
		T_ASSERT((indexEndOffset - indexOffset) % 3 == 0);

		m_primitives[lod] = render::Primitives::setIndexed(
			render::PrimitiveType::Triangles,
			(uint32_t)indexOffset,
			(uint32_t)(indexEndOffset - indexOffset) / 3);
	}

	m_indexBuffer = m_renderSystem->createBuffer(
		render::BuIndex,
		(uint32_t)indices.size() * sizeof(uint32_t),
		false,
		T_FILE_LINE_W);
	if (!m_indexBuffer)
		return false;

	uint32_t* index = static_cast< uint32_t* >(m_indexBuffer->lock());
	T_ASSERT_M(index, L"Unable to lock index buffer");

	for (uint32_t i = 0; i < uint32_t(indices.size()); ++i)
		index[i] = indices[i];

	m_indexBuffer->unlock();

	return true;
}

bool TerrainComponent::createTiles()
{
	// Tile being loaded is about to be replaced.
	if (m_loadJob)
	{
		m_loadJob->wait();
		m_loadJob = nullptr;
		m_loadingTile = -1;
	}

	for (auto& tile : m_tiles)
	{
		safeDestroy(tile.drawBuffer);
		safeDestroy(tile.dataBuffer);
	}
	m_tiles.clear();

	safeDestroy(m_overview.drawBuffer);
	safeDestroy(m_overview.dataBuffer);
	m_overview = Tile();

	m_primed = false;

	const Terrain* terrain = m_terrain;
	const hf::Heightfield* overviewHeightfield = terrain->getOverview().heightfield;
	if (!overviewHeightfield || terrain->getTileCount() <= 0 || terrain->getTileSize() <= 0)
	{
		log::error << L"Unable to create terrain; no tiles, terrain need to be rebuilt." << Endl;
		return false;
	}

	const int32_t cellsPerPatch = (terrain->getPatchDim() - 1) * terrain->getDetailSkip();
	const int32_t tileCount = terrain->getTileCount();
	const int32_t tileSize = terrain->getTileSize();
	const Vector4 worldExtent = overviewHeightfield->getWorldExtent();
	const float cellSizeX = worldExtent.x() / (tileSize * tileCount);
	const float cellSizeZ = worldExtent.z() / (tileSize * tileCount);
	const float tileSizeX = tileSize * cellSizeX;
	const float tileSizeZ = tileSize * cellSizeZ;

	// Tile maps cover its samples and the seam, first patch vertex at first sample.
	m_tiles.resize(tileCount * tileCount);
	for (int32_t tz = 0; tz < tileCount; ++tz)
	{
		for (int32_t tx = 0; tx < tileCount; ++tx)
		{
			Tile& tile = m_tiles[tx + tz * tileCount];
			const float x0 = -worldExtent.x() * 0.5f + tx * tileSizeX;
			const float z0 = -worldExtent.z() * 0.5f + tz * tileSizeZ;
			tile.worldOrigin = Vector4(x0, -worldExtent.y() * 0.5f, z0, 0.0f);
			tile.worldExtent = Vector4((tileSize + 1) * cellSizeX, worldExtent.y(), (tileSize + 1) * cellSizeZ, 0.0f);
			tile.patchOrigin = Vector4(x0 + cellSizeX * 0.5f, -worldExtent.y() * 0.5f, z0 + cellSizeZ * 0.5f, 1.0f);
			tile.patchExtent = Vector4(cellsPerPatch * cellSizeX, worldExtent.y(), cellsPerPatch * cellSizeZ, 0.0f);
			tile.boundingBox = Aabb3(
				Vector4(x0, -worldExtent.y() * 0.5f, z0, 1.0f),
				Vector4(x0 + tileSizeX, worldExtent.y() * 0.5f, z0 + tileSizeZ, 1.0f));
			tile.patchCount = std::max(tileSize / cellsPerPatch, 1);
			tile.debugIndex = tx + tz * tileCount;
		}
	}

	// Overview patches are about as dense as overview's samples, each patch belong to a single tile.
	const float overviewCellSize = worldExtent.x() / overviewHeightfield->getSize();
	const int32_t overviewPatchesPerTile = std::max((int32_t)(tileSizeX / (overviewCellSize * cellsPerPatch) + 0.5f), 1);

	m_overview.resources = terrain->getOverview();
	m_overview.worldOrigin = (-worldExtent * 0.5_simd).xyz0();
	m_overview.worldExtent = worldExtent;
	m_overview.patchOrigin = Vector4(-worldExtent.x() * 0.5f + cellSizeX * 0.5f, -worldExtent.y() * 0.5f, -worldExtent.z() * 0.5f + cellSizeZ * 0.5f, 1.0f);
	m_overview.patchExtent = Vector4(tileSizeX / overviewPatchesPerTile, worldExtent.y(), tileSizeZ / overviewPatchesPerTile, 0.0f);
	m_overview.patchCount = tileCount * overviewPatchesPerTile;
	m_overview.debugIndex = 0;
	m_overview.boundingBox = Aabb3(-worldExtent * 0.5_simd, worldExtent * 0.5_simd);
	m_overview.loaded = true;

	m_overview.patchTiles.resize(m_overview.patchCount * m_overview.patchCount);
	for (int32_t pz = 0; pz < m_overview.patchCount; ++pz)
		for (int32_t px = 0; px < m_overview.patchCount; ++px)
			m_overview.patchTiles[px + pz * m_overview.patchCount] = (px / overviewPatchesPerTile) + (pz / overviewPatchesPerTile) * tileCount;

	float gridX, gridZ;
	overviewHeightfield->worldToGrid(m_overview.patchOrigin.x(), m_overview.patchOrigin.z(), gridX, gridZ);
	updatePatches(m_overview, overviewHeightfield, gridX, gridZ, m_overview.patchExtent.x() / overviewCellSize, nullptr);

	return createTileBuffers(m_overview);
}

bool TerrainComponent::createTileBuffers(Tile& tile)
{
	const uint32_t patchCount = tile.patchCount * tile.patchCount;

	for (auto& view : tile.view)
		view.viewPatches.resize(patchCount);

	// Buffers are kept when tile is unloaded since they might still be in flight.
	if (tile.drawBuffer)
		return true;

	const uint32_t alignedPatchCount = alignUp(patchCount, 16);

	tile.drawBuffer = m_renderSystem->createBuffer(
		render::BuStructured | render::BuIndirect,
		alignedPatchCount * sizeof(render::IndexedIndirectDraw),
		true,
		T_FILE_LINE_W);
	if (!tile.drawBuffer)
		return false;

	tile.dataBuffer = m_renderSystem->createBuffer(
		render::BuStructured,
		alignedPatchCount * sizeof(DrawData),
		true,
		T_FILE_LINE_W);
	if (!tile.dataBuffer)
		return false;

	return true;
}

void TerrainComponent::updatePatches(Tile& tile, const hf::Heightfield* heightfield, float gridX, float gridZ, float gridSize, const int32_t* region)
{
	const uint32_t patchDim = m_terrain->getPatchDim();
	const int32_t patchCount = tile.patchCount;

	tile.patches.resize(patchCount * patchCount);

	const int32_t mnx = region ? clamp(region[0], 0, patchCount - 1) : 0;
	const int32_t mnz = region ? clamp(region[1], 0, patchCount - 1) : 0;
	const int32_t mxx = region ? clamp(region[2], 0, patchCount - 1) : patchCount - 1;
	const int32_t mxz = region ? clamp(region[3], 0, patchCount - 1) : patchCount - 1;

	for (int32_t pz = mnz; pz <= mxz; ++pz)
	{
		for (int32_t px = mnx; px <= mxx; ++px)
		{
			const float gx = gridX + px * gridSize;
			const float gz = gridZ + pz * gridSize;

			float heights[2];
			float errors[LodCount - 1];
			calculatePatchMinMaxHeight(heightfield, gx, gz, gridSize, heights);
			calculatePatchErrorMetrics(heightfield, LodCount, gx, gz, gridSize, patchDim, errors);

			Patch& patch = tile.patches[px + pz * patchCount];
			patch.minHeight = heights[0];
			patch.maxHeight = heights[1];
			patch.error[0] = 0.0f;
			for (int32_t i = 1; i < LodCount; ++i)
				patch.error[i] = errors[i - 1];
		}
	}
}

void TerrainComponent::loadTile(int32_t tileIndex)
{
	Tile& tile = m_tiles[tileIndex];
	if (!m_terrain->bindTile(m_resourceManager, tileIndex, tile.resources))
	{
		log::error << L"Unable to load terrain tile " << tileIndex << L"." << Endl;
		tile.failed = true;
		return;
	}

	const float cellsPerPatch = (float)((m_terrain->getPatchDim() - 1) * m_terrain->getDetailSkip());
	updatePatches(tile, tile.resources.heightfield, 0.0f, 0.0f, cellsPerPatch, nullptr);
}

void TerrainComponent::unloadTile(Tile& tile)
{
	tile.resources = Terrain::TileResources();
	tile.patches.clear();
	tile.loaded = false;
	tile.revision = ++m_revision;
}

void TerrainComponent::updateStreaming(const Vector4& eyePosition)
{
	// Tiles initially in range are loaded immediately, as if they were part of terrain.
	if (!m_primed)
	{
		for (int32_t i = 0; i < (int32_t)m_tiles.size(); ++i)
		{
			Tile& tile = m_tiles[i];
			if (tile.loaded || tile.failed || tile.pinned || distanceToTile(eyePosition, tile.boundingBox) >= c_tileLoadDistance)
				continue;

			loadTile(i);
			tile.loaded = !tile.patches.empty() && createTileBuffers(tile);
			tile.revision = ++m_revision;
		}
		m_primed = true;
		return;
	}

	// Finish loading tile, or discard it if no longer wanted.
	if (m_loadJob && m_loadJob->wait(0))
	{
		Tile& tile = m_tiles[m_loadingTile];
		if (!tile.pinned)
		{
			if (!tile.patches.empty() && distanceToTile(eyePosition, tile.boundingBox) <= c_tileUnloadDistance && createTileBuffers(tile))
			{
				tile.loaded = true;
				tile.revision = ++m_revision;
			}
			else
				unloadTile(tile);
		}
		m_loadJob = nullptr;
		m_loadingTile = -1;
	}

	// Unload tiles beyond unload distance and find nearest tile to load.
	float nearestDistance = c_tileLoadDistance;
	int32_t nearestTile = -1;
	int32_t unloadCount = 0;

	for (int32_t i = 0; i < (int32_t)m_tiles.size(); ++i)
	{
		Tile& tile = m_tiles[i];
		if (tile.failed || tile.pinned || i == m_loadingTile)
			continue;

		const float distance = distanceToTile(eyePosition, tile.boundingBox);
		if (tile.loaded)
		{
			if (distance > c_tileUnloadDistance)
			{
				unloadTile(tile);
				++unloadCount;
			}
		}
		else if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestTile = i;
		}
	}

	// Load one tile at a time in background.
	if (!m_loadJob && nearestTile >= 0)
	{
		m_loadingTile = nearestTile;
		m_loadJob = JobManager::getInstance().add([this, nearestTile]() {
			loadTile(nearestTile);
		});
	}

	// Release resources of unloaded tiles.
	if (unloadCount > 0)
		m_resourceManager->unloadUnusedResident();
}

void TerrainComponent::editTile(int32_t tileIndex, render::ITexture* heightMap, render::ITexture* normalMap, render::ITexture* splatMap, render::ITexture* colorMap, render::ITexture* cutMap)
{
	// Tile being loaded would overwrite edited maps.
	if (m_loadJob && m_loadingTile == tileIndex)
	{
		m_loadJob->wait();
		m_loadJob = nullptr;
		m_loadingTile = -1;
	}

	Tile& tile = m_tiles[tileIndex];

	// Edited tile's heightfield is used to calculate it's patches.
	if (!tile.resources.heightfield)
	{
		Terrain::TileResources resources;
		if (m_terrain->bindTile(m_resourceManager, tileIndex, resources))
			tile.resources.heightfield = resources.heightfield;
	}

	tile.resources.heightMap = resource::Proxy< render::ITexture >(heightMap);
	tile.resources.normalMap = resource::Proxy< render::ITexture >(normalMap);
	tile.resources.splatMap = resource::Proxy< render::ITexture >(splatMap);
	tile.resources.colorMap = resource::Proxy< render::ITexture >(colorMap);
	tile.resources.cutMap = resource::Proxy< render::ITexture >(cutMap);
	tile.pinned = true;
	tile.failed = false;

	updateTilePatches(tileIndex, nullptr);
	tile.loaded = !tile.patches.empty() && createTileBuffers(tile);
	tile.revision = ++m_revision;
}

void TerrainComponent::updateTilePatches(int32_t tileIndex, const int32_t* gridRegion)
{
	Tile& tile = m_tiles[tileIndex];

	const hf::Heightfield* heightfield = tile.resources.heightfield;
	if (!heightfield)
		return;

	const int32_t tileCount = m_terrain->getTileCount();
	const int32_t tileSize = m_terrain->getTileSize();
	const int32_t cellsPerPatch = (m_terrain->getPatchDim() - 1) * m_terrain->getDetailSkip();
	const int32_t gridX = (tileIndex % tileCount) * tileSize;
	const int32_t gridZ = (tileIndex / tileCount) * tileSize;

	// Samples on patch borders belong to both patches.
	if (gridRegion)
	{
		const int32_t region[] = {
			(gridRegion[0] - gridX - 1) / cellsPerPatch,
			(gridRegion[1] - gridZ - 1) / cellsPerPatch,
			(gridRegion[2] - gridX) / cellsPerPatch,
			(gridRegion[3] - gridZ) / cellsPerPatch
		};
		updatePatches(tile, heightfield, 0.0f, 0.0f, (float)cellsPerPatch, region);
	}
	else
		updatePatches(tile, heightfield, 0.0f, 0.0f, (float)cellsPerPatch, nullptr);

	tile.revision = ++m_revision;
}

void TerrainComponent::setupTile(Tile& tile, int32_t viewIndex, const world::WorldRenderView& worldRenderView, const Frustum& worldCullFrustum, bool snapshot)
{
	View& view = tile.view[viewIndex];

	const Vector4& worldExtent = m_overview.worldExtent;
	const Matrix44 viewProj = worldRenderView.getProjection() * worldRenderView.getView();
	const Vector4 eyePosition = worldRenderView.getEyePosition();
	const Frustum viewCullFrustum = worldRenderView.getCullFrustum();

	const Vector4 patchExtent = tile.patchExtent;
	const Vector4 patchDeltaHalf = patchExtent * Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	const Vector4 patchDeltaX = patchExtent * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	const Vector4 patchDeltaZ = patchExtent * Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 patchTopLeft = tile.patchOrigin;

	// Cull patches to world frustum.
	AlignedVector< CullPatch >& visiblePatches = view.visiblePatches;
	visiblePatches.resize(0);

	for (int32_t pz = 0; pz < tile.patchCount; ++pz)
	{
		Vector4 patchOrigin = patchTopLeft;
		for (int32_t px = 0; px < tile.patchCount; ++px)
		{
			const uint32_t patchId = px + pz * tile.patchCount;
			ViewPatch& viewPatch = view.viewPatches[patchId];

			// Overview patches are only rendered where tile isn't loaded.
			if (!tile.patchTiles.empty() && m_tiles[tile.patchTiles[patchId]].loaded)
			{
				viewPatch.lastPatchLod = c_patchLodSteps;
				patchOrigin += patchDeltaX;
				continue;
			}

			const Patch& patch = tile.patches[patchId];

			const Vector4 patchCenterWorld = (patchOrigin + patchDeltaHalf) * Vector4(1.0f, 0.0f, 1.0f, 0.0f) + Vector4(0.0f, (patch.minHeight + patch.maxHeight) * 0.5f, 0.0f, 1.0f);

			const Aabb3 patchAabb(
				patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + Vector4(-patchDeltaHalf.x(), patch.minHeight, -patchDeltaHalf.z(), 0.0f),
				patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + Vector4(patchDeltaHalf.x(), patch.maxHeight, patchDeltaHalf.z(), 0.0f));

			if (worldCullFrustum.inside(patchAabb) != Frustum::Result::Outside)
			{
				const Scalar lodDistance = (patchCenterWorld - eyePosition).xyz0().length();

				const Vector4 patchCenterWorld_x0zw = patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f);
				const Vector4 eyePosition_0y00 = Vector4(0.0f, eyePosition.y(), 0.0f, 0.0f);

				CullPatch cp;

				// Calculate screen error for each lod.
				for (int i = 0; i < LodCount; ++i)
				{
					const Vector4 Pworld[2] = {
						patchCenterWorld_x0zw + eyePosition_0y00,
						patchCenterWorld_x0zw + eyePosition_0y00 + Vector4(0.0f, patch.error[i], 0.0f, 0.0f)
					};

					Vector4 Pview[2] = {
						worldRenderView.getView() * Pworld[0],
						worldRenderView.getView() * Pworld[1]
					};

					if (Pview[0].z() < viewCullFrustum.getNearZ())
						Pview[0].set(2, viewCullFrustum.getNearZ());
					if (Pview[1].z() < viewCullFrustum.getNearZ())
						Pview[1].set(2, viewCullFrustum.getNearZ());

					Vector4 Pclip[] = {
						worldRenderView.getProjection() * Pview[0].xyz1(),
						worldRenderView.getProjection() * Pview[1].xyz1()
					};

					T_ASSERT(Pclip[0].w() > 0.0f);
					T_ASSERT(Pclip[1].w() > 0.0f);

					Pclip[0] /= Pclip[0].w();
					Pclip[1] /= Pclip[1].w();

					const Vector4 d = Pclip[1] - Pclip[0];

					const float dx = d.x();
					const float dy = d.y();

					cp.error[i] = std::sqrt(dx * dx + dy * dy) * 100.0f;
				}

				// Project patch bounding box extents onto view plane and calculate screen area.
				Vector4 extents[8];
				patchAabb.getExtents(extents);

				Vector4 mn(
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max());
				Vector4 mx(
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max());

				bool clipped = false;
				for (int32_t i = 0; i < sizeof_array(extents); ++i)
				{
					Vector4 p = viewProj * extents[i];
					if (p.w() <= 0.0f)
					{
						clipped = true;
						break;
					}

					// Homogeneous divide.
					p /= p.w();

					// Track screen space extents.
					mn = min(mn, p);
					mx = max(mx, p);
				}

				const Vector4 e = mx - mn;

				cp.distance = lodDistance;
				cp.area = !clipped ? e.x() * e.y() : 1000.0_simd;
				cp.patchId = patchId;
				cp.patchOrigin = patchOrigin;
				cp.patchAabb = patchAabb;

				visiblePatches.push_back(cp);
			}
			else
				viewPatch.lastPatchLod = c_patchLodSteps;

			patchOrigin += patchDeltaX;
		}
		patchTopLeft += patchDeltaZ;
	}

	// Sort patches front to back to maximize best use of surface cache and rendering.
	std::sort(visiblePatches.begin(), visiblePatches.end(), [](const CullPatch& lh, const CullPatch& rh) {
		return lh.distance < rh.distance;
	});

	for (uint32_t i = 0; i < LodCount; ++i)
		view.patchLodInstances[i].resize(0);

	// Find patch lod based on screen space error.
	for (const auto& visiblePatch : visiblePatches)
	{
		ViewPatch& viewPatch = view.viewPatches[visiblePatch.patchId];

		int32_t patchLod = 0;
		for (int32_t j = 3; j > 0; --j)
		{
			if (visiblePatch.error[j] <= 1.0f)
			{
				patchLod = j;
				break;
			}
		}

		viewPatch.lastPatchLod = patchLod;
		viewPatch.surfaceOffset = Vector4::zero();

		view.patchLodInstances[patchLod].push_back(&visiblePatch);
	}

	if (visiblePatches.empty())
		return;

	// Update indirect draw buffers.
	auto draw = (render::IndexedIndirectDraw*)tile.drawBuffer->lock();
	auto data = (DrawData*)tile.dataBuffer->lock();

	for (const auto& visiblePatch : visiblePatches)
	{
		const Vector4& patchOrigin = visiblePatch.patchOrigin;
		const ViewPatch& viewPatch = view.viewPatches[visiblePatch.patchId];

		const auto& p = m_primitives[viewPatch.lastPatchLod];
		draw->indexCount = p.getVertexCount();
		draw->instanceCount = 1;
		draw->firstIndex = p.offset;
		draw->vertexOffset = 0;
		draw->firstInstance = 0;
		draw++;

		patchOrigin.storeUnaligned(data->patchOrigin);

		if (!snapshot)
			viewPatch.surfaceOffset.storeUnaligned(data->surfaceOffset);
		else
		{
			const Vector4 snapshotOffset(
				patchOrigin.x() / worldExtent.x() + 0.5f,
				patchOrigin.z() / worldExtent.z() + 0.5f,
				patchExtent.x() / worldExtent.x(),
				patchExtent.z() / worldExtent.z());
			snapshotOffset.storeUnaligned(data->surfaceOffset);
		}

		visiblePatch.patchAabb.mn.storeAligned(data->patchBoundingBoxMn);
		visiblePatch.patchAabb.mx.storeAligned(data->patchBoundingBoxMx);
		data++;
	}

	tile.drawBuffer->unlock();
	tile.dataBuffer->unlock();
}

void TerrainComponent::buildTile(const Tile& tile, int32_t viewIndex, const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	const View& view = tile.view[viewIndex];
	if (view.visiblePatches.empty())
		return;

	render::ITexture* cutMap = tile.resources.cutMap;
	render::ITexture* colorMap = tile.resources.colorMap;

	render::Shader* shader = m_terrain->getTerrainShader();
	auto perm = worldRenderPass.getPermutation(shader);
	shader->setCombination(c_handleTerrain_CutEnable, cutMap != nullptr && cutMap->getSize().x > 1, perm);
	if (m_visualizeMode == VmPatchLod || m_visualizeMode == VmTile)
		shader->setCombination(c_handleTerrain_VisualizeLods, true, perm);
	else if (m_visualizeMode >= VmColorMap && m_visualizeMode <= VmCutMap)
		shader->setCombination(c_handleTerrain_VisualizeMap, true, perm);

	render::IProgram* program = shader->getProgram(perm).program;
	if (!program)
		return;

	render::RenderContext* renderContext = context.getRenderContext();

	// Render all visible patches using indirect draw.
	auto rb = renderContext->allocNamed< render::IndirectRenderBlock >(L"Terrain patches");
	rb->distance = 10000.0f;
	rb->program = program;
	rb->programParams = renderContext->alloc< render::ProgramParameters >();
	rb->indexBuffer = m_indexBuffer->getBufferView();
	rb->indexType = render::IndexType::UInt32;
	rb->vertexBuffer = m_vertexBuffer->getBufferView();
	rb->vertexLayout = m_vertexLayout;
	rb->primitive = render::PrimitiveType::Triangles;
	rb->drawBuffer = tile.drawBuffer->getBufferView();
	rb->drawCount = (uint32_t)view.visiblePatches.size();

	rb->programParams->beginParameters(renderContext);

	rb->programParams->setTextureParameter(c_handleTerrain_Heightfield, tile.resources.heightMap);
	rb->programParams->setTextureParameter(c_handleTerrain_SurfaceAlbedo, m_surfaceCache->getBaseTexture());
	rb->programParams->setTextureParameter(c_handleTerrain_ColorMap, colorMap ? colorMap : m_defaultColorMap.ptr());
	rb->programParams->setTextureParameter(c_handleTerrain_Normals, tile.resources.normalMap);
	rb->programParams->setTextureParameter(c_handleTerrain_SplatMap, tile.resources.splatMap);
	rb->programParams->setTextureParameter(c_handleTerrain_CutMap, cutMap ? cutMap : m_defaultCutMap.ptr());
	rb->programParams->setVectorParameter(c_handleTerrain_Eye, worldRenderView.getEyePosition());
	rb->programParams->setVectorParameter(c_handleTerrain_WorldOrigin, tile.worldOrigin);
	rb->programParams->setVectorParameter(c_handleTerrain_WorldExtent, tile.worldExtent);
	rb->programParams->setVectorParameter(c_handleTerrain_PatchExtent, tile.patchExtent);

	// Scale and offset from tile texture coordinates into terrain texture coordinates.
	const Vector4& terrainOrigin = m_overview.worldOrigin;
	const Vector4& terrainExtent = m_overview.worldExtent;
	rb->programParams->setVectorParameter(c_handleTerrain_TileTransform, Vector4(
		tile.worldExtent.x() / terrainExtent.x(),
		tile.worldExtent.z() / terrainExtent.z(),
		(tile.worldOrigin.x() - terrainOrigin.x()) / terrainExtent.x(),
		(tile.worldOrigin.z() - terrainOrigin.z()) / terrainExtent.z()));

	if (m_visualizeMode == VmColorMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, colorMap ? colorMap : m_defaultColorMap.ptr());
	else if (m_visualizeMode == VmNormalMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, tile.resources.normalMap);
	else if (m_visualizeMode == VmHeightMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, tile.resources.heightMap);
	else if (m_visualizeMode == VmSplatMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, tile.resources.splatMap);
	else if (m_visualizeMode == VmCutMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, cutMap ? cutMap : m_defaultCutMap.ptr());

	if (m_visualizeMode == VmPatchLod)
		rb->programParams->setVectorParameter(c_handleTerrain_DebugPatchIndex, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
	else if (m_visualizeMode == VmTile)
		rb->programParams->setVectorParameter(c_handleTerrain_DebugPatchIndex, Vector4(tile.debugIndex % 4, 0.0f, 0.0f, 0.0f));

	worldRenderPass.setProgramParameters(rb->programParams);

	rb->programParams->setBufferViewParameter(c_handleTerrain_PatchData, tile.dataBuffer->getBufferView());
	rb->programParams->endParameters(renderContext);

	renderContext->draw(render::RenderPriority::Opaque, rb);
}

const hf::Heightfield* TerrainComponent::getHeightfield(float worldX, float worldZ) const
{
	const int32_t tileCount = m_terrain->getTileCount();
	if ((int32_t)m_tiles.size() == tileCount * tileCount)
	{
		const Vector4& worldExtent = m_overview.worldExtent;
		const int32_t tx = clamp((int32_t)((worldX / worldExtent.x() + 0.5f) * tileCount), 0, tileCount - 1);
		const int32_t tz = clamp((int32_t)((worldZ / worldExtent.z() + 0.5f) * tileCount), 0, tileCount - 1);
		const Tile& tile = m_tiles[tx + tz * tileCount];
		if (tile.loaded && tile.resources.heightfield)
			return tile.resources.heightfield;
	}
	return m_overview.resources.heightfield;
}

void TerrainComponent::updateRayTracingPatches()
{
	if (!m_renderSystem->supportRayTracing() || m_rtTiles.size() != m_tiles.size())
		return;

	bool changed = false;
	for (int32_t i = 0; i < (int32_t)m_tiles.size(); ++i)
	{
		if (m_rtTiles[i].revision == m_tiles[i].revision)
			continue;

		createRayTracingTile(i);
		m_rtTiles[i].revision = m_tiles[i].revision;
		changed = true;
	}
	if (!changed)
		return;

	m_rtParts.resize(0);
	for (const auto& rtTile : m_rtTiles)
		m_rtParts.insert(m_rtParts.end(), rtTile.parts.begin(), rtTile.parts.end());

	// Reset world to create new RT instance.
	setWorld(m_world);
}

bool TerrainComponent::createRayTracingPatches()
{
	if (!m_renderSystem->supportRayTracing())
		return true;

	if (!m_rtVertexLayout)
	{
		AlignedVector< render::VertexElement > vertexElements;
		vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat3, 0));
		m_rtVertexLayout = m_renderSystem->createVertexLayout(vertexElements);
		if (!m_rtVertexLayout)
			return false;
	}

	// All patches are triangulated alike, two triangles per cell, thus share index buffer.
	if (!m_rtIndexBuffer)
	{
		const uint32_t patchVertexDim = c_rtPatchCells + 1;

		m_rtIndexBuffer = m_renderSystem->createBuffer(
			render::BuIndex,
			c_rtPatchCells * c_rtPatchCells * 6 * sizeof(uint32_t),
			false,
			T_FILE_LINE_W);
		if (!m_rtIndexBuffer)
			return false;

		uint32_t* index = static_cast< uint32_t* >(m_rtIndexBuffer->lock());
		T_ASSERT_M(index, L"Unable to lock index buffer");
		for (uint32_t z = 0; z < c_rtPatchCells; ++z)
		{
			for (uint32_t x = 0; x < c_rtPatchCells; ++x)
			{
				const uint32_t offset = x + z * patchVertexDim;

				*index++ = offset;
				*index++ = offset + 1;
				*index++ = offset + patchVertexDim;

				*index++ = offset + 1;
				*index++ = offset + 1 + patchVertexDim;
				*index++ = offset + patchVertexDim;
			}
		}
		m_rtIndexBuffer->unlock();
	}

	// Patches of tiles are created by next update.
	safeDestroy(m_rtwInstance);
	m_rtParts.resize(0);
	m_rtTiles.resize(0);
	m_rtTiles.resize(m_tiles.size());
	return true;
}

bool TerrainComponent::createRayTracingTile(int32_t tileIndex)
{
	RtTile& rtTile = m_rtTiles[tileIndex];
	rtTile.vertexBuffers.resize(0);
	rtTile.parts.resize(0);

	const Tile& tile = m_tiles[tileIndex];
	const Vector4& worldExtent = m_overview.worldExtent;
	const float tileExtentX = tile.boundingBox.mx.x() - tile.boundingBox.mn.x();
	const float tileExtentZ = tile.boundingBox.mx.z() - tile.boundingBox.mn.z();

	// Loaded tiles are full resolution, unloaded tiles are a single coarse patch from overview.
	const uint32_t patchCount = tile.loaded ? std::max< uint32_t >((uint32_t)(tileExtentX / (c_rtPatchCells * c_rtCellSize) + 0.5f), 1) : 1;
	const uint32_t patchVertexDim = c_rtPatchCells + 1;
	const uint32_t patchVertexCount = patchVertexDim * patchVertexDim;
	const uint32_t patchTriangleCount = c_rtPatchCells * c_rtPatchCells * 2;

	// Vertices are placed on a single grid spanning all patches so neighbouring patches share border vertices.
	const float cellSizeX = tileExtentX / float(patchCount * c_rtPatchCells);
	const float cellSizeZ = tileExtentZ / float(patchCount * c_rtPatchCells);
	const float originX = -worldExtent.x() * 0.5f;
	const float originZ = -worldExtent.z() * 0.5f;

	// Shrink the RT terrain a bit to reduce self intersection due to mismatch between GBuffer and RT geometry.
	const Scalar elevationOffsetXZ = -1.0_simd;
	const Scalar elevationOffsetY = -0.5_simd;

	const auto albedoMap = (m_surfaceCache != nullptr && m_surfaceCache->getBaseTexture() != nullptr) ? m_surfaceCache->getBaseTexture()->getBindlessIndex() : -1;

	const render::Primitives primitives = render::Primitives::setIndexed(
		render::PrimitiveType::Triangles,
		0,
		patchTriangleCount);

	// Attributes are fetched per triangle corner; normals and texture coordinates are calculated per vertex and then gathered.
	AlignedVector< Vector4 > normals(patchVertexCount);
	AlignedVector< float > texCoords(patchVertexCount * 2);

	for (uint32_t pz = 0; pz < patchCount; ++pz)
	{
		for (uint32_t px = 0; px < patchCount; ++px)
		{
			Ref< render::Buffer > vertexBuffer = m_renderSystem->createBuffer(
				render::BuVertex,
				patchVertexCount * 3 * sizeof(float),
				false,
				T_FILE_LINE_W);
			if (!vertexBuffer)
				return false;

			float* vertex = static_cast< float* >(vertexBuffer->lock());
			T_ASSERT_M(vertex, L"Unable to lock vertex buffer");
			for (uint32_t z = 0; z < patchVertexDim; ++z)
			{
				for (uint32_t x = 0; x < patchVertexDim; ++x)
				{
					const uint32_t i = x + z * patchVertexDim;

					const float worldX = tile.boundingBox.mn.x() + float(px * c_rtPatchCells + x) * cellSizeX;
					const float worldZ = tile.boundingBox.mn.z() + float(pz * c_rtPatchCells + z) * cellSizeZ;

					const hf::Heightfield* heightfield = getHeightfield(worldX, worldZ);
					const float worldY = heightfield->getWorldHeight(worldX, worldZ);

					float gridX, gridZ;
					heightfield->worldToGrid(worldX, worldZ, gridX, gridZ);

					const Vector4 normal = heightfield->normalAt(gridX, gridZ);

					normals[i] = normal;
					texCoords[i * 2 + 0] = (worldX - originX) / worldExtent.x();
					texCoords[i * 2 + 1] = (worldZ - originZ) / worldExtent.z();

					*vertex++ = worldX + normal.x() * elevationOffsetXZ;
					*vertex++ = worldY + normal.y() * elevationOffsetY;
					*vertex++ = worldZ + normal.z() * elevationOffsetXZ;
				}
			}
			vertexBuffer->unlock();

			// Create "per-vertex" attribute buffer, one attribute per triangle corner.
			Ref< render::Buffer > perVertexData = m_renderSystem->createBuffer(
				render::BuStructured,
				patchTriangleCount * 3 * sizeof(world::HWRT_Material),
				false,
				T_FILE_LINE_W);
			if (!perVertexData)
				return false;

			world::HWRT_Material* va = static_cast< world::HWRT_Material* >(perVertexData->lock());
			T_ASSERT_M(va, L"Unable to lock vertex attribute buffer");
			for (uint32_t cz = 0; cz < c_rtPatchCells; ++cz)
			{
				for (uint32_t cx = 0; cx < c_rtPatchCells; ++cx)
				{
					const uint32_t offset = cx + cz * patchVertexDim;
					const uint32_t corners[] = { offset, offset + 1, offset + patchVertexDim, offset + 1, offset + 1 + patchVertexDim, offset + patchVertexDim };
					for (uint32_t index : corners)
					{
						Vector4(0.2f, 0.4f, 0.1f, 0.0f).storeUnaligned3(va->albedo);
						normals[index].storeUnaligned3(va->normal);
						va->emissive = 0.0f;
						va->texCoord[0] = texCoords[index * 2 + 0];
						va->texCoord[1] = texCoords[index * 2 + 1];
						va->albedoMap = albedoMap;
						va++;
					}
				}
			}
			perVertexData->unlock();

			world::RTWorldComponent::Part& part = rtTile.parts.push_back();
			part.perVertexData = perVertexData;
			part.blas = m_renderSystem->createAccelerationStructure(vertexBuffer, m_rtVertexLayout, m_rtIndexBuffer, render::IndexType::UInt32, { { primitives, true } }, false, false);
			if (!part.blas)
				return false;

			rtTile.vertexBuffers.push_back(vertexBuffer);
		}
	}

	return true;
}

}
