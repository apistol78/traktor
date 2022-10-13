#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
#include "Render/Types.h"
#include "Terrain/TerrainSurfaceAlloc.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;
class IRenderTargetSet;
class ScreenRenderer;
class RenderContext;
class RenderBlock;
class RenderGraph;

}

namespace traktor::terrain
{

class Terrain;

/*! Terrain surface cache manager.
 * \ingroup Terrain
 */
class T_DLLCLASS TerrainSurfaceCache : public Object
{
	T_RTTI_CLASS;

public:
	TerrainSurfaceCache();

	virtual ~TerrainSurfaceCache();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, uint32_t size);

	void destroy();

	void flush(uint32_t patchId);

	void flush();

	void setupBaseColor(
		render::RenderGraph& renderGraph,
		Terrain* terrain,
		const Vector4& worldOrigin,
		const Vector4& worldExtent
	);

	void setupPatch(
		render::RenderGraph& renderGraph,
		Terrain* terrain,
		const Vector4& worldOrigin,
		const Vector4& worldExtent,
		const Vector4& patchOrigin,
		const Vector4& patchExtent,
		uint32_t surfaceLod,
		uint32_t patchId,
		// Out
		Vector4& outTextureOffset
	);

	render::ISimpleTexture* getVirtualAlbedo() const;

	render::ISimpleTexture* getVirtualNormals() const;

	render::ISimpleTexture* getBaseTexture() const;

private:
	struct Entry
	{
		uint32_t lod;
		TerrainSurfaceAlloc::Tile tile;
	};

	resource::IResourceManager* m_resourceManager;
	render::IRenderSystem* m_renderSystem;
	Ref< render::ScreenRenderer > m_screenRenderer;
	TerrainSurfaceAlloc m_alloc;
	Ref< render::IRenderTargetSet > m_virtualTexture;
	Ref< render::IRenderTargetSet > m_base;
	AlignedVector< Entry > m_entries;
	bool m_haveBase = false;
	bool m_clearCache = true;
	uint32_t m_updateCount = 0;
};

}
