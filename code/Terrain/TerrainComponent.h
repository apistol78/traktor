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
#include "World/Entity/RTWorldComponent.h"

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
		VmCutMap = 7
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

	const AlignedVector< Patch >& getPatches() const { return m_patches; }

	uint32_t getPatchCount() const { return m_patchCount; }

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setWorld(world::World* world) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	friend class TerrainEditModifier;
	friend class TerrainComponentEditor;

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
		int32_t lastSurfaceLod = 0;
		Vector4 surfaceOffset = Vector4::zero();
	};

	struct View
	{
		AlignedVector< ViewPatch > viewPatches;
		AlignedVector< CullPatch > visiblePatches;
		AlignedVector< const CullPatch* > patchLodInstances[LodCount];
	};

	world::World* m_world = nullptr;
	world::Entity* m_owner = nullptr;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	resource::Proxy< Terrain > m_terrain;
	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< render::Shader > m_shaderCull;
	AlignedVector< Patch > m_patches;
	uint32_t m_patchCount = 0;
	uint32_t m_cacheSize = 0;
	Ref< TerrainSurfaceCache > m_surfaceCache;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_indexBuffer;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_drawBuffer;
	Ref< render::Buffer > m_culledDrawBuffer;
	Ref< render::Buffer > m_dataBuffer;
	Ref< render::ITexture > m_defaultColorMap;
	Ref< render::ITexture > m_defaultCutMap;
	render::Primitives m_primitives[LodCount];
	AlignedVector< uint32_t > m_indices;
	RefArray< render::Buffer > m_rtVertexBuffers;
	AlignedVector< world::RTWorldComponent::Part > m_rtParts;
	world::RTWorldComponent::Instance* m_rtwInstance = nullptr;
	float m_patchLodDistance = 0.0f;
	float m_patchLodBias = 0.0f;
	float m_patchLodExponent = 0.0f;
	float m_surfaceLodDistance = 0.0f;
	float m_surfaceLodBias = 0.0f;
	float m_surfaceLodExponent = 0.0f;
	VisualizeMode m_visualizeMode = VmDefault;
	View m_view[4];

	bool validate(int32_t viewIndex, uint32_t cacheSize);

	void updatePatches(const uint32_t* region, bool updateErrors);

	void updateRayTracingPatches();

	bool createPatches();

	bool createRayTracingPatches();
};

}
