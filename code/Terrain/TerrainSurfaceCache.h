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

	namespace render
	{

class RenderSystem;
class RenderTargetSet;
class ScreenRenderer;
class Texture;
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

	bool create(render::RenderSystem* renderSystem);

	void destroy();

	void flush(uint32_t patchId);

	void flush();

	void get(
		const world::WorldRenderView* worldRenderView,
		render::RenderContext* renderContext,
		TerrainSurface* surface,
		render::Texture* heightfieldTexture,
		const Vector4& worldOrigin,
		const Vector4& worldExtent,
		const Vector4& patchOrigin,
		const Vector4& patchExtent,
		uint32_t surfaceLod,
		uint32_t patchId,
		// Out
		render::RenderBlock*& outRenderBlock,
		Ref< render::Texture >& outTexture
	);

private:
	struct Entry
	{
		uint32_t lod;
		Ref< render::RenderTargetSet > renderTargetSet;
	};

	Ref< render::RenderSystem > m_renderSystem;
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
