#ifndef traktor_terrain_TerrainEntity_H
#define traktor_terrain_TerrainEntity_H

#include "Resource/Proxy.h"
#include "World/Entity/Entity.h"
#include "Render/Shader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class WorldRenderView;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class RenderContext;
class VertexBuffer;
class IndexBuffer;
class ITexture;

	}

	namespace terrain
	{

class Heightfield;
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

	TerrainEntity();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const TerrainEntityData& data);

	void render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView);

	inline const resource::Proxy< Heightfield >& getHeightfield() const { return m_heightfield; }

	inline Ref< TerrainSurfaceCache > getSurfaceCache() const { return m_surfaceCache; }

	virtual void update(const world::EntityUpdate* update);

private:
	resource::Proxy< Heightfield > m_heightfield;
	resource::Proxy< render::Shader > m_shader;
	Ref< TerrainSurfaceCache > m_surfaceCache;
	Ref< TerrainSurface > m_surface;
	std::vector< Patch > m_patches;
	uint32_t m_patchCount;
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
};

	}
}

#endif	// traktor_terrain_TerrainEntity_H
