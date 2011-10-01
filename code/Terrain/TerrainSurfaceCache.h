#ifndef traktor_terrain_TerrainSurfaceCache_H
#define traktor_terrain_TerrainSurfaceCache_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class ISimpleTexture;
class RenderTargetSet;
class ScreenRenderer;
class RenderContext;
class RenderBlock;

	}

	namespace terrain
	{

class TerrainSurface;

class T_DLLCLASS TerrainSurfaceCache : public Object
{
	T_RTTI_CLASS;

public:
	TerrainSurfaceCache();

	virtual ~TerrainSurfaceCache();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	void destroy();

	void flush(uint32_t patchId);

	void flush();

	void get(
		render::RenderContext* renderContext,
		TerrainSurface* surface,
		render::ISimpleTexture* heightfieldTexture,
		const Vector4& worldOrigin,
		const Vector4& worldExtent,
		const Vector4& patchOrigin,
		const Vector4& patchExtent,
		uint32_t surfaceLod,
		uint32_t patchId,
		// Out
		render::RenderBlock*& outRenderBlock,
		Ref< render::ISimpleTexture >& outTexture
	);

private:
	struct Entry
	{
		uint32_t lod;
		Ref< render::RenderTargetSet > renderTargetSet;
	};

	resource::IResourceManager* m_resourceManager;
	render::IRenderSystem* m_renderSystem;
	Ref< render::ScreenRenderer > m_screenRenderer;
	RefArray< render::RenderTargetSet > m_cache[4];
	std::vector< Entry > m_entries;
	render::handle_t m_handleHeightfield;
	render::handle_t m_handleHeightfieldSize;
	render::handle_t m_handleWorldOrigin;
	render::handle_t m_handleWorldExtent;
	render::handle_t m_handlePatchOrigin;
	render::handle_t m_handlePatchExtent;
	render::handle_t m_handlePatchLodColor;

	Ref< render::RenderTargetSet > allocateTarget(uint32_t lod);
};

	}
}

#endif	// traktor_terrain_TerrainSurfaceCache_H
