#ifndef traktor_terrain_TerrainEntity_H
#define traktor_terrain_TerrainEntity_H

#include "Render/Shader.h"
#include "Resource/Proxy.h"
#include "Terrain/TerrainEntityData.h"
#include "World/Entity/Entity.h"

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
class MaterialMask;

	}

	namespace render
	{

class IRenderSystem;
class ISimpleTexture;
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
class WorldRenderView;

	}

	namespace terrain
	{

class TerrainSurfaceCache;
class TerrainSurface;

#define T_USE_TERRAIN_VERTEX_TEXTURE_FETCH

class T_DLLCLASS TerrainEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	struct Patch
	{
		float minHeight;
		float maxHeight;
#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
		Ref< render::VertexBuffer > vertexBuffer;
#endif
		int32_t lastPatchLod;
		int32_t lastSurfaceLod;
	};

	TerrainEntity(render::IRenderSystem* renderSystem, bool editorMode);

	bool create(resource::IResourceManager* resourceManager, const TerrainEntityData& data);

	void render(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

	const resource::Proxy< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const resource::Proxy< hf::MaterialMask >& getMaterialMask() const { return m_materialMask; }

	TerrainSurfaceCache* getSurfaceCache() const { return m_surfaceCache; }

	const std::vector< Patch >& getPatches() const { return m_patches; }

	uint32_t getPatchCount() const { return m_patchCount; }

	virtual void update(const world::EntityUpdate* update);

private:
	friend class TerrainEntityEditor;

	Ref< render::IRenderSystem > m_renderSystem;
	bool m_editorMode;
	TerrainEntityData::VisualizeMode m_visualizeMode;
	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< hf::MaterialMask > m_materialMask;
	resource::Proxy< render::Shader > m_shader;
	Ref< TerrainSurfaceCache > m_surfaceCache;
	Ref< TerrainSurface > m_surface;
	std::vector< Patch > m_patches;
	uint32_t m_patchCount;
	Ref< render::ISimpleTexture > m_normalTexture;
	Ref< render::ISimpleTexture > m_heightTexture;
	Ref< render::ISimpleTexture > m_materialMaskTexture;
	Ref< render::IndexBuffer > m_indexBuffer;
#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	Ref< render::VertexBuffer > m_vertexBuffer;
#endif
	render::Primitives m_primitives[4];
	float m_patchLodDistance;
	float m_patchLodBias;
	float m_patchLodExponent;
	float m_surfaceLodDistance;
	float m_surfaceLodBias;
	float m_surfaceLodExponent;
	render::handle_t m_handleSurface;
	render::handle_t m_handleHeightfield;
	render::handle_t m_handleHeightfieldSize;
	render::handle_t m_handleNormals;
	render::handle_t m_handleNormalsSize;
	render::handle_t m_handleWorldOrigin;
	render::handle_t m_handleWorldExtent;
	render::handle_t m_handlePatchOrigin;
	render::handle_t m_handlePatchExtent;
	render::handle_t m_handlePatchLodColor;

	bool updatePatches(int32_t minX, int32_t minZ, int32_t maxX, int32_t maxZ);

	bool createPatches();

	bool updateTextures(bool normals, bool heights, bool materials);

	bool createTextures();
};

	}
}

#endif	// traktor_terrain_TerrainEntity_H
