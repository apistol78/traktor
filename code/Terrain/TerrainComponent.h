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

namespace traktor
{
	namespace hf
	{

class Heightfield;

	}

	namespace render
	{

class IRenderSystem;
class ISimpleTexture;
class ITexture;
class RenderContext;
class VertexBuffer;
class IndexBuffer;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;
class WorldSetupContext;

	}

	namespace terrain
	{

class TerrainSurfaceCache;

//#if !TARGET_OS_IPHONE
#	define T_USE_TERRAIN_VERTEX_TEXTURE_FETCH
//#endif

/*! Terrain entity.
 * \ingroup Terrain
 */
class T_DLLCLASS TerrainComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	enum
	{
		LodCount = 4
	};

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
#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
		Ref< render::VertexBuffer > vertexBuffer;
#endif
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

	TerrainComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

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
	Ref< render::IndexBuffer > m_indexBuffer;
#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	Ref< render::VertexBuffer > m_vertexBuffer;
#endif
	render::Primitives m_primitives[LodCount];
	float m_patchLodDistance;
	float m_patchLodBias;
	float m_patchLodExponent;
	float m_surfaceLodDistance;
	float m_surfaceLodBias;
	float m_surfaceLodExponent;
	VisualizeMode m_visualizeMode;
	AlignedVector< CullPatch > m_visiblePatches;
#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	 AlignedVector< const CullPatch* > m_patchLodInstances[LodCount];
#endif

	bool validate(int32_t viewIndex, uint32_t cacheSize);

	void updatePatches(const uint32_t* region);

	bool createPatches();
};

	}
}

