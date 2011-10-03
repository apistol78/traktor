#ifndef traktor_terrain_TerrainEntity_H
#define traktor_terrain_TerrainEntity_H

#include "Resource/Proxy.h"
#include "World/Entity/Entity.h"
#include "Render/Shader.h"

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

class TerrainEntityData;
class TerrainSurfaceCache;
class TerrainSurface;

//#define T_USE_TERRAIN_VERTEX_TEXTURE_FETCH

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
	};

	TerrainEntity(render::IRenderSystem* renderSystem, bool editorMode);

	bool create(resource::IResourceManager* resourceManager, const TerrainEntityData& data);

	void render(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

	inline const resource::Proxy< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	inline Ref< TerrainSurfaceCache > getSurfaceCache() const { return m_surfaceCache; }

	virtual void update(const world::EntityUpdate* update);

private:
	friend class TerrainEntityEditor;

	Ref< render::IRenderSystem > m_renderSystem;
	bool m_editorMode;
	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< render::Shader > m_shader;
	Ref< TerrainSurfaceCache > m_surfaceCache;
	Ref< TerrainSurface > m_surface;
	std::vector< Patch > m_patches;
	uint32_t m_patchCount;
	Ref< render::ISimpleTexture > m_normalTexture;
	Ref< render::ISimpleTexture > m_heightTexture;
	Ref< render::IndexBuffer > m_indexBuffer;
#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	Ref< render::VertexBuffer > m_vertexBuffer;
#endif
	render::Primitives m_primitives[4];
	float m_patchLodDistance;
	float m_surfaceLodDistance;
	render::handle_t m_handleSurface;
	render::handle_t m_handleHeightfield;
	render::handle_t m_handleHeightfieldSize;
	render::handle_t m_handleNormals;
	render::handle_t m_handleNormalsSize;
	render::handle_t m_handleWorldOrigin;
	render::handle_t m_handleWorldExtent;
	render::handle_t m_handlePatchOrigin;
	render::handle_t m_handlePatchExtent;

	bool updatePatches(int32_t minX, int32_t minZ, int32_t maxX, int32_t maxZ);

	bool createPatches();

	bool updateTextures(bool normals, bool heights);

	bool createTextures();
};

	}
}

#endif	// traktor_terrain_TerrainEntity_H
