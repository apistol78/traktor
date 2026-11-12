/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Frustum.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponentData.h"
#include "World/IEntityComponent.h"
#include "World/Entity/RTWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

}

namespace traktor::hf
{

class Heightfield;

}

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class ITexture;
class IVertexLayout;
class RenderContext;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;
class WorldSetupContext;

}

namespace traktor::terrain
{

class TerrainSurfaceCache;

/*! Terrain entity.
 * \ingroup Terrain
 *
 * Tiles near the camera are streamed in and rendered at full resolution,
 * remaining tiles are rendered from the low resolution overview.
 */
class T_DLLCLASS TerrainComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	static constexpr int32_t LodCount = 4;

	enum VisualizeMode
	{
		VmDefault = 0,
		VmPatchLod = 1,
		VmColorMap = 2,
		VmNormalMap = 3,
		VmHeightMap = 4,
		VmSplatMap = 5,
		VmCutMap = 6
	};

	struct Patch
	{
		float minHeight;
		float maxHeight;
		float error[LodCount];
	};

	explicit TerrainComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	bool create(const TerrainComponentData& data);

	void setup(
		const world::WorldSetupContext& context,
		const world::WorldRenderView& worldRenderView,
		float detailDistance,
		uint32_t cacheSize
	);

	void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		float detailDistance,
		uint32_t cacheSize
	);

	void setVisualizeMode(VisualizeMode visualizeMode);

	const resource::Proxy< Terrain >& getTerrain() const { return m_terrain; }

	TerrainSurfaceCache* getSurfaceCache() const { return m_surfaceCache; }

	//! Intersect ray with loaded tiles, or overview where tiles aren't loaded.
	bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const;

	//! Height at world position from loaded tile, or overview if tile isn't loaded.
	float getWorldHeight(float worldX, float worldZ) const;

	//! Number of tiles, loaded or not, in row major order.
	int32_t getTileCount() const { return (int32_t)m_tiles.size(); }

	//! Heightfield of tile, null if tile isn't loaded.
	const hf::Heightfield* getTileHeightfield(int32_t tileIndex) const;

	//! Revision of tile, changed each time tile is loaded, unloaded or it's patches are updated.
	uint32_t getTileRevision(int32_t tileIndex) const { return m_tiles[tileIndex].revision; }

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setWorld(world::World* world) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	friend class TerrainEditModifier;

	struct CullPatch
	{
		float error[4];
		float distance;
		float area;
		uint32_t patchId;
		Vector4 patchOrigin;
		Aabb3 patchAabb;
	};

	struct ViewPatch
	{
		int32_t lastPatchLod = 0;
		Vector4 surfaceOffset = Vector4::zero();
	};

	struct View
	{
		AlignedVector< ViewPatch > viewPatches;
		AlignedVector< CullPatch > visiblePatches;
		AlignedVector< const CullPatch* > patchLodInstances[LodCount];
	};

	//! Patches, and resources to render them, of a tile or of the overview.
	struct Tile
	{
		Terrain::TileResources resources;
		Vector4 worldOrigin;					//!< Origin of area covered by tile's maps.
		Vector4 worldExtent;					//!< Extent of area covered by tile's maps.
		Vector4 patchOrigin;					//!< Position of first vertex of first patch.
		Vector4 patchExtent;					//!< Extent of a single patch.
		Aabb3 boundingBox;						//!< Area covered by tile's patches, used for streaming.
		int32_t patchCount = 0;					//!< Number of patches along each side.
		AlignedVector< Patch > patches;
		AlignedVector< int32_t > patchTiles;	//!< Tile covered by each overview patch.
		View view[4];
		Ref< render::Buffer > drawBuffer;
		Ref< render::Buffer > dataBuffer;
		bool loaded = false;
		bool failed = false;					//!< Unable to load tile, not retried.
		bool pinned = false;					//!< Edited tile, ignored by streaming.
		uint32_t revision = 0;
	};

	//! Ray tracing patches of a tile, full resolution if tile is loaded else a coarse patch from overview.
	struct RtTile
	{
		uint32_t revision = ~0U;	//!< Revision of tile when patches were created.
		RefArray< render::Buffer > vertexBuffers;
		AlignedVector< world::RTWorldComponent::Part > parts;
	};

	world::World* m_world = nullptr;
	world::Entity* m_owner = nullptr;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	resource::Proxy< Terrain > m_terrain;
	uint32_t m_cacheSize = 0;
	Ref< TerrainSurfaceCache > m_surfaceCache;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_indexBuffer;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::ITexture > m_defaultColorMap;
	Ref< render::ITexture > m_defaultCutMap;
	render::Primitives m_primitives[LodCount];
	Ref< const render::IVertexLayout > m_rtVertexLayout;
	Ref< render::Buffer > m_rtIndexBuffer;
	AlignedVector< RtTile > m_rtTiles;
	AlignedVector< world::RTWorldComponent::Part > m_rtParts;
	world::RTWorldComponent::Instance* m_rtwInstance = nullptr;
	VisualizeMode m_visualizeMode = VmDefault;
	AlignedVector< Tile > m_tiles;
	Tile m_overview;
	Ref< Job > m_loadJob;
	int32_t m_loadingTile = -1;
	uint32_t m_revision = 0;
	bool m_primed = false;

	bool validate(int32_t viewIndex, uint32_t cacheSize);

	bool createGeometry();

	bool createTiles();

	bool createTileBuffers(Tile& tile);

	/*! Calculate patches of tile from heightfield, first patch vertex at grid x, z and each patch covering grid size.
	 * Region is range of patches, x0, z0, x1, z1 inclusive, or null for all patches.
	 */
	void updatePatches(Tile& tile, const hf::Heightfield* heightfield, float gridX, float gridZ, float gridSize, const int32_t* region);

	void loadTile(int32_t tileIndex);

	void unloadTile(Tile& tile);

	void updateStreaming(const Vector4& eyePosition);

	//! Replace maps of tile with edited maps; tile is pinned and kept loaded.
	void editTile(int32_t tileIndex, render::ITexture* heightMap, render::ITexture* normalMap, render::ITexture* splatMap, render::ITexture* colorMap, render::ITexture* cutMap);

	//! Update patches of tile intersecting grid region, x0, z0, x1, z1 inclusive in grid of entire terrain, from tile's heightfield.
	void updateTilePatches(int32_t tileIndex, const int32_t* gridRegion);

	void setupTile(Tile& tile, int32_t viewIndex, const world::WorldRenderView& worldRenderView, const Frustum& worldCullFrustum, bool snapshot);

	void buildTile(const Tile& tile, int32_t viewIndex, const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass);

	//! Heightfield of loaded tile at world position, or overview if tile isn't loaded.
	const hf::Heightfield* getHeightfield(float worldX, float worldZ) const;

	//! Create ray tracing patches of tiles which have changed since last update.
	void updateRayTracingPatches();

	//! Reset ray tracing patches; patches of every tile are created by next update.
	bool createRayTracingPatches();

	bool createRayTracingTile(int32_t tileIndex);
};

}
