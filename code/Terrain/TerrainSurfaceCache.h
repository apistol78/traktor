#ifndef traktor_terrain_TerrainSurfaceCache_H
#define traktor_terrain_TerrainSurfaceCache_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Render/Types.h"

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
class RenderTargetSet;
class ScreenRenderer;
class ITexture;
class RenderContext;
class RenderBlock;

	}

	namespace terrain
	{

class TerrainSurface;
class Heightfield;

class T_DLLCLASS TerrainSurfaceCache : public Object
{
	T_RTTI_CLASS(TerrainSurfaceCache)

public:
	TerrainSurfaceCache();

	virtual ~TerrainSurfaceCache();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	void destroy();

	void flush(uint32_t patchId);

	void flush();

	void get(
		const world::WorldRenderView* worldRenderView,
		render::RenderContext* renderContext,
		TerrainSurface* surface,
		render::ITexture* heightfieldTexture,
		const Vector4& worldOrigin,
		const Vector4& worldExtent,
		const Vector4& patchOrigin,
		const Vector4& patchExtent,
		uint32_t surfaceLod,
		uint32_t patchId,
		// Out
		render::RenderBlock*& outRenderBlock,
		Ref< render::ITexture >& outTexture
	);

private:
	struct Entry
	{
		uint32_t lod;
		Ref< render::RenderTargetSet > renderTargetSet;
	};

	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::ScreenRenderer > m_screenRenderer;
	RefArray< render::RenderTargetSet > m_cache[4];
	std::vector< Entry > m_entries;
	render::handle_t m_handleHeightfield;
	render::handle_t m_handleHeightfieldSize;
	render::handle_t m_handleWorldOrigin;
	render::handle_t m_handleWorldExtent;
	render::handle_t m_handlePatchOrigin;
	render::handle_t m_handlePatchExtent;

	render::RenderTargetSet* allocateTarget(uint32_t lod);
};

	}
}

#endif	// traktor_terrain_TerrainSurfaceCache_H
