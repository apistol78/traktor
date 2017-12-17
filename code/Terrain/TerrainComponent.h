/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_terrain_TerrainComponent_H
#define traktor_terrain_TerrainComponent_H

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
class WorldContext;
class WorldRenderView;

	}

	namespace terrain
	{

class ITerrainLayer;
class TerrainSurfaceCache;

//#if !TARGET_OS_IPHONE
//#	define T_USE_TERRAIN_VERTEX_TEXTURE_FETCH
//#endif

/*! \brief Terrain entity.
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

	TerrainComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	bool create(const TerrainComponentData& data);

	void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float detailDistance,
		uint32_t cacheSize,
		bool layersEnable
	);

	void setVisualizeMode(VisualizeMode visualizeMode);

	const resource::Proxy< Terrain >& getTerrain() const { return m_terrain; }

	TerrainSurfaceCache* getSurfaceCache() const { return m_surfaceCache; }

	const AlignedVector< Patch >& getPatches() const { return m_patches; }

	uint32_t getPatchCount() const { return m_patchCount; }

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setOwner(world::Entity* owner) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

private:
	friend class TerrainEditModifier;
	friend class TerrainComponentEditor;

	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	resource::Proxy< Terrain > m_terrain;
	resource::Proxy< hf::Heightfield > m_heightfield;
	Ref< TerrainSurfaceCache > m_surfaceCache;
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
	RefArray< ITerrainLayer > m_layers;

	render::handle_t m_handleSurface;
	render::handle_t m_handleSurfaceOffset;
	render::handle_t m_handleHeightfield;
	render::handle_t m_handleColorMap;
	render::handle_t m_handleSplatMap;
	render::handle_t m_handleCutMap;
	render::handle_t m_handleMaterialMap;
	render::handle_t m_handleNormals;
	render::handle_t m_handleEye;
	render::handle_t m_handleWorldOrigin;
	render::handle_t m_handleWorldExtent;
	render::handle_t m_handlePatchOrigin;
	render::handle_t m_handlePatchExtent;
	render::handle_t m_handleDetailDistance;
	render::handle_t m_handleDebugPatchColor;
	render::handle_t m_handleDebugMap;
	render::handle_t m_handleCutEnable;
	render::handle_t m_handleColorEnable;

	bool updatePatches();

	bool createPatches();
};

	}
}

#endif	// traktor_terrain_TerrainComponent_H
