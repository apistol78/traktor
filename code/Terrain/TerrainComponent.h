/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"
#include "Terrain/TerrainComponentData.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

class Heightfield;

}

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class ISimpleTexture;
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
 */
class T_DLLCLASS TerrainComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	static constexpr int32_t LodCount = 4;

	enum VisualizeMode
	{
		VmDefault = 0,
		VmSurfaceLod = 1,
		VmPatchLod = 2,
		VmColorMap = 3,
		VmNormalMap = 4,
		VmHeightMap = 5,
		VmSplatMap = 6,
		VmCutMap = 7,
		VmMaterialMap = 8
	};

	struct Patch
	{
		float minHeight;
		float maxHeight;
		float error[LodCount];
		int32_t lastPatchLod;
		int32_t lastSurfaceLod;
		Vector4 surfaceOffset;
	};

	struct CullPatch
	{
		float error[4];
		float distance;
		float area;
		uint32_t patchId;
		Vector4 patchOrigin;
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

	TerrainSurfaceCache* getSurfaceCache(int32_t viewIndex) const { return m_surfaceCache[viewIndex]; }

	const AlignedVector< Patch >& getPatches() const { return m_patches; }

	uint32_t getPatchCount() const { return m_patchCount; }

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	friend class TerrainEditModifier;
	friend class TerrainComponentEditor;

	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	world::Entity* m_owner;
	resource::Proxy< Terrain > m_terrain;
	resource::Proxy< hf::Heightfield > m_heightfield;
	Ref< TerrainSurfaceCache > m_surfaceCache[4];
	AlignedVector< Patch > m_patches;
	uint32_t m_patchCount;
	uint32_t m_cacheSize;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_indexBuffer;
	Ref< render::Buffer > m_vertexBuffer;
	render::Primitives m_primitives[LodCount];
	float m_patchLodDistance;
	float m_patchLodBias;
	float m_patchLodExponent;
	float m_surfaceLodDistance;
	float m_surfaceLodBias;
	float m_surfaceLodExponent;
	VisualizeMode m_visualizeMode;
	AlignedVector< CullPatch > m_visiblePatches;
	AlignedVector< const CullPatch* > m_patchLodInstances[LodCount];

	bool validate(int32_t viewIndex, uint32_t cacheSize);

	void updatePatches(const uint32_t* region);

	bool createPatches();
};

}
