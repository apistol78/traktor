#ifndef traktor_terrain_TerrainSurfaceCache_H
#define traktor_terrain_TerrainSurfaceCache_H

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

class Terrain;

/*! \brief Terrain surface cache manager.
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

	void flushBase();

	void begin(
		render::RenderContext* renderContext,
		Terrain* terrain,
		const Vector4& worldOrigin,
		const Vector4& worldExtent
	);

	void get(
		render::RenderContext* renderContext,
		Terrain* terrain,
		const Vector4& worldOrigin,
		const Vector4& worldExtent,
		const Vector4& patchOrigin,
		const Vector4& patchExtent,
		uint32_t surfaceLod,
		uint32_t patchId,
		// Out
		render::RenderBlock*& outRenderBlock,
		Vector4& outTextureOffset
	);

	render::ISimpleTexture* getVirtualTexture() const;

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
	Ref< render::RenderTargetSet > m_pool;
	Ref< render::RenderTargetSet > m_base;
	std::vector< Entry > m_entries;
	bool m_clearCache;
	uint32_t m_updateCount;
	uint32_t m_size;
	render::handle_t m_handleHeightfield;
	render::handle_t m_handleColorMap;
	render::handle_t m_handleSplatMap;
	render::handle_t m_handleWorldOrigin;
	render::handle_t m_handleWorldExtent;
	render::handle_t m_handlePatchOrigin;
	render::handle_t m_handlePatchExtent;
	render::handle_t m_handleTextureOffset;
};

	}
}

#endif	// traktor_terrain_TerrainSurfaceCache_H
