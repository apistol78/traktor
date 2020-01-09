#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const uint32_t c_maxUpdatePerFrame = 1;
const int32_t c_margin = 1;

struct TerrainSurfaceRenderBlock : public render::DrawableRenderBlock
{
	render::ScreenRenderer* screenRenderer;

	TerrainSurfaceRenderBlock()
	:	screenRenderer(nullptr)
	{
	}

	virtual void render(render::IRenderView* renderView) const
	{
		if (programParams)
			programParams->fixup(program);

		screenRenderer->draw(renderView, program);
	}
};

Vector4 offsetFromTile(const TerrainSurfaceAlloc::Tile& tile, uint32_t size)
{
	return Vector4(
		tile.x / 4096.0f + float(c_margin) / size,
		tile.y / 4096.0f + float(c_margin) / size,
		tile.dim / 4096.0f - float(2.0f * c_margin) / size,
		tile.dim / 4096.0f - float(2.0f * c_margin) / size
	);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainSurfaceCache", TerrainSurfaceCache, Object)

TerrainSurfaceCache::TerrainSurfaceCache()
:	m_haveBase(false)
,	m_clearCache(true)
,	m_updateCount(0)
,	m_size(0)
,	m_handleColorEnable(render::getParameterHandle(L"ColorEnable"))
,	m_handleHeightfield(render::getParameterHandle(L"Heightfield"))
,	m_handleColorMap(render::getParameterHandle(L"ColorMap"))
,	m_handleSplatMap(render::getParameterHandle(L"SplatMap"))
,	m_handleWorldOrigin(render::getParameterHandle(L"WorldOrigin"))
,	m_handleWorldExtent(render::getParameterHandle(L"WorldExtent"))
,	m_handlePatchOrigin(render::getParameterHandle(L"PatchOrigin"))
,	m_handlePatchExtent(render::getParameterHandle(L"PatchExtent"))
,	m_handleTextureOffset(render::getParameterHandle(L"TextureOffset"))
{
}

TerrainSurfaceCache::~TerrainSurfaceCache()
{
	destroy();
}

bool TerrainSurfaceCache::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, uint32_t size)
{
	m_resourceManager = resourceManager;
	m_renderSystem = renderSystem;

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	render::RenderTargetSetCreateDesc desc;
	desc.count = 1;
	desc.width = size;
	desc.height = size;
	desc.multiSample = 0;
	desc.createDepthStencil = false;
	desc.usingPrimaryDepthStencil = false;
	desc.targets[0].format = render::TfR8G8B8A8;

	m_pool = renderSystem->createRenderTargetSet(desc, T_FILE_LINE_W);
	if (!m_pool)
		return false;

	desc.count = 1;
#if !defined(__ANDROID__) && !defined(__IOS__)
	desc.width = 256;
	desc.height = 256;
#else
	desc.width = 64;
	desc.height = 64;
#endif
	desc.multiSample = 0;
	desc.createDepthStencil = false;
	desc.usingPrimaryDepthStencil = false;
	desc.targets[0].format = render::TfR8G8B8A8;

	m_base = renderSystem->createRenderTargetSet(desc, T_FILE_LINE_W);
	if (!m_base)
		return false;

	m_haveBase = false;
	m_clearCache = true;
	m_updateCount = 0;
	m_size = size;
	return true;
}

void TerrainSurfaceCache::destroy()
{
	if (m_entries.empty())
		return;

	flush();

	safeDestroy(m_pool);
	safeDestroy(m_screenRenderer);
}

void TerrainSurfaceCache::flush(uint32_t patchId)
{
	if (patchId >= m_entries.size())
		return;

	Entry& entry = m_entries[patchId];
	if (entry.tile.dim > 0)
	{
		m_alloc.free(entry.tile);
		entry.tile.dim = 0;
	}
}

void TerrainSurfaceCache::flush()
{
	for (uint32_t i = 0; i < m_entries.size(); ++i)
		flush(i);

	m_entries.resize(0);
}

void TerrainSurfaceCache::begin(
	render::RenderContext* renderContext,
	Terrain* terrain,
	const Vector4& worldOrigin,
	const Vector4& worldExtent
)
{
	const static Vector4 c_textureOffset(-1.0f, 1.0f, 2.0f, -2.0f);

	if (!m_haveBase)
	{
		render::Shader* shader = terrain->getSurfaceShader();
		if (!shader)
			return;

		render::ISimpleTexture* heightMap = terrain->getHeightMap();
		render::ISimpleTexture* colorMap = terrain->getColorMap();
		render::ISimpleTexture* splatMap = terrain->getSplatMap();

		shader->setCombination(m_handleColorEnable, colorMap != nullptr);

		if (!shader->getCurrentProgram())
			return;

		auto tb = renderContext->alloc< render::TargetBeginRenderBlock >("Terrain surface, base begin");
		tb->renderTargetSet = m_base;
		tb->renderTargetIndex = -1;
		tb->clear.mask = render::CfColor;
		tb->clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
		renderContext->enqueue(tb);

		auto rb = renderContext->alloc< TerrainSurfaceRenderBlock >("Terrain surface, base");
		rb->screenRenderer = m_screenRenderer;
		rb->distance = 0.0f;
		rb->program = shader->getCurrentProgram();
		rb->programParams = renderContext->alloc< render::ProgramParameters >();
		rb->programParams->beginParameters(renderContext);
		rb->programParams->setTextureParameter(m_handleHeightfield, heightMap);
		rb->programParams->setTextureParameter(m_handleColorMap, colorMap);
		rb->programParams->setTextureParameter(m_handleSplatMap, splatMap);
		rb->programParams->setVectorParameter(m_handleWorldOrigin, worldOrigin);
		rb->programParams->setVectorParameter(m_handleWorldExtent, worldExtent);
		rb->programParams->setVectorParameter(m_handlePatchOrigin, worldOrigin);
		rb->programParams->setVectorParameter(m_handlePatchExtent, worldExtent);
		rb->programParams->setVectorParameter(m_handleTextureOffset, c_textureOffset);
		rb->programParams->endParameters(renderContext);
		renderContext->enqueue(rb);

		auto te = renderContext->alloc< render::TargetEndRenderBlock >("Terrain surface, base end");
		renderContext->enqueue(te);

		m_haveBase = true;
	}

	m_updateCount = 0;
}

void TerrainSurfaceCache::get(
	render::RenderContext* renderContext,
	Terrain* terrain,
	const Vector4& worldOrigin,
	const Vector4& worldExtent,
	const Vector4& patchOrigin,
	const Vector4& patchExtent,
	uint32_t surfaceLod,
	uint32_t patchId,
	// Out
	Vector4& outTextureOffset
)
{
	// If the cache is already valid we just reuse it.
	if (patchId < m_entries.size())
	{
		if (m_updateCount >= c_maxUpdatePerFrame || (m_entries[patchId].lod == surfaceLod && m_entries[patchId].tile.dim > 0))
		{
			outTextureOffset = offsetFromTile(m_entries[patchId].tile, m_size);
			return;
		}

		// Release cache as it's no longer valid.
		flush(patchId);
	}
	else
	{
		// Patch hasn't been cached before, allocate a new entry.
		m_entries.resize(patchId + 1);
	}

	++m_updateCount;

	// Allocate tile for this patch; first try to allocate proper size
	// then fall back on smaller and smaller tiles.
	TerrainSurfaceAlloc::Tile tile;
	for (uint32_t i = surfaceLod; i < 4; ++i)
	{
		if (m_alloc.alloc(i, tile))
			break;
	}
	if (!tile.dim)
	{
		outTextureOffset = Vector4::zero();
		T_DEBUG(L"Unable to allocate terrain surface tile; out of memory");
		return;
	}

	Vector4 worldOriginM = worldOrigin;
	Vector4 worldExtentM = worldExtent;

	// Number of patch units per texel.
	float dpx = patchExtent.x() / (m_size * (tile.dim / 4096.0f));
	float dpz = patchExtent.z() / (m_size * (tile.dim / 4096.0f));

	Vector4 patchOriginM = patchOrigin;
	Vector4 patchExtentM = patchExtent;

	patchOriginM -= Vector4(dpx, 0.0f, dpz, 0.0f) * Scalar(c_margin);
	patchExtentM += Vector4(dpx, 0.0f, dpz, 0.0f) * Scalar(2.0f * c_margin);

	Vector4 textureOffset(
		-1.0f + 2.0f * tile.x / 4096.0f,
		 1.0f - 2.0f * tile.y / 4096.0f,
		 2.0f * tile.dim / 4096.0f,
		-2.0f * tile.dim / 4096.0f
	);

	render::Shader* shader = terrain->getSurfaceShader();
	if (!shader)
		return;

	render::ISimpleTexture* heightMap = terrain->getHeightMap();
	render::ISimpleTexture* colorMap = terrain->getColorMap();
	render::ISimpleTexture* splatMap = terrain->getSplatMap();

	shader->setCombination(m_handleColorEnable, colorMap != nullptr);

	if (!shader->getCurrentProgram())
		return;

	auto tb = renderContext->alloc< render::TargetBeginRenderBlock >("Terrain surface, begin");
	tb->renderTargetSet = m_pool;
	tb->renderTargetIndex = -1;
	tb->clear.mask = 0;
	if (m_clearCache)
	{
		tb->clear.mask = render::CfColor;
		tb->clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	}
	renderContext->enqueue(tb);

	auto rb = renderContext->alloc< TerrainSurfaceRenderBlock >("Terrain surface");
	rb->screenRenderer = m_screenRenderer;
	rb->distance = 0.0f;
	rb->program = shader->getCurrentProgram();
	rb->programParams = renderContext->alloc< render::ProgramParameters >();
	rb->programParams->beginParameters(renderContext);
	rb->programParams->setTextureParameter(m_handleHeightfield, heightMap);
	rb->programParams->setTextureParameter(m_handleColorMap, colorMap);
	rb->programParams->setTextureParameter(m_handleSplatMap, splatMap);
	rb->programParams->setVectorParameter(m_handleWorldOrigin, worldOriginM);
	rb->programParams->setVectorParameter(m_handleWorldExtent, worldExtentM);
	rb->programParams->setVectorParameter(m_handlePatchOrigin, patchOriginM);
	rb->programParams->setVectorParameter(m_handlePatchExtent, patchExtentM);
	rb->programParams->setVectorParameter(m_handleTextureOffset, textureOffset);
	rb->programParams->endParameters(renderContext);
	renderContext->enqueue(rb);

	auto te = renderContext->alloc< render::TargetEndRenderBlock >("Terrain surface, end");
	renderContext->enqueue(te);

	m_clearCache = false;

	// Update cache entry.
	m_entries[patchId].lod = surfaceLod;
	m_entries[patchId].tile = tile;

	outTextureOffset = offsetFromTile(tile, m_size);
}

render::ISimpleTexture* TerrainSurfaceCache::getVirtualTexture() const
{
	return m_pool->getColorTexture(0);
}

render::ISimpleTexture* TerrainSurfaceCache::getBaseTexture() const
{
	return m_base->getColorTexture(0);
}

	}
}
