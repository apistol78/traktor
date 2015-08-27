#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
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

//const uint32_t c_maxUpdatePerFrame = 1;
const int32_t c_margin = 1;

struct TerrainSurfaceRenderBlock : public render::RenderBlock
{
	render::ScreenRenderer* screenRenderer;
	render::RenderTargetSet* renderTargetSet;
	TerrainSurfaceRenderBlock* next;
	bool clear;

	TerrainSurfaceRenderBlock()
	:	screenRenderer(0)
	,	renderTargetSet(0)
	,	next(0)
	,	clear(false)
	{
	}

	virtual void render(render::IRenderView* renderView, const render::ProgramParameters* globalParameters) const
	{
		if (globalParameters)
			globalParameters->fixup(program);
		if (programParams)
			programParams->fixup(program);

		if (renderTargetSet)
		{
			renderView->begin(renderTargetSet, 0);
			if (clear)
			{
				const static Color4f cc(0.0f, 0.0f, 0.0f, 0.0f);
				renderView->clear(render::CfColor, &cc, 1.0f, 0);
			}
		}
		
		screenRenderer->draw(renderView, program);

		if (next)
			next->render(renderView, globalParameters);

		if (renderTargetSet)
			renderView->end();
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
:	m_clearCache(true)
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

	m_pool = renderSystem->createRenderTargetSet(desc);
	if (!m_pool)
		return false;

	desc.count = 1;
	desc.width = 256;
	desc.height = 256;
	desc.multiSample = 0;
	desc.createDepthStencil = false;
	desc.usingPrimaryDepthStencil = false;
	desc.targets[0].format = render::TfR8G8B8A8;

	m_base = renderSystem->createRenderTargetSet(desc);
	if (!m_base)
		return false;

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

void TerrainSurfaceCache::flushBase()
{
	if (m_base)
		m_base->setContentValid(false);
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
	if (!m_pool->isContentValid())
		flush();

	if (!m_base->isContentValid())
	{
		render::Shader* shader = terrain->getSurfaceShader();
		if (!shader)
			return;

		render::ISimpleTexture* heightMap = terrain->getHeightMap();
		render::ISimpleTexture* colorMap = terrain->getColorMap();
		render::ISimpleTexture* splatMap = terrain->getSplatMap();

		shader->setCombination(L"ColorEnable", colorMap != 0);

		if (!shader->getCurrentProgram())
			return;

		TerrainSurfaceRenderBlock* renderBlock = renderContext->alloc< TerrainSurfaceRenderBlock >("Terrain surface (base)");

		renderBlock->screenRenderer = m_screenRenderer;
		renderBlock->distance = 0.0f;
		renderBlock->program = shader->getCurrentProgram();
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();

		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setTextureParameter(m_handleHeightfield, heightMap);
		renderBlock->programParams->setTextureParameter(m_handleColorMap, colorMap);
		renderBlock->programParams->setTextureParameter(m_handleSplatMap, splatMap);
		renderBlock->programParams->setVectorParameter(m_handleWorldOrigin, worldOrigin);
		renderBlock->programParams->setVectorParameter(m_handleWorldExtent, worldExtent);
		renderBlock->programParams->setVectorParameter(m_handlePatchOrigin, worldOrigin);
		renderBlock->programParams->setVectorParameter(m_handlePatchExtent, worldExtent);

		const Vector4 textureOffset(-1.0f, 1.0f, 2.0f, -2.0f);
		renderBlock->programParams->setVectorParameter(m_handleTextureOffset, textureOffset);
		renderBlock->programParams->endParameters(renderContext);

		renderBlock->renderTargetSet = m_base;
		renderBlock->clear = true;

		renderContext->draw(render::RpOpaque, renderBlock);

		m_base->setContentValid(true);
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
	render::RenderBlock*& outRenderBlock,
	Vector4& outTextureOffset
)
{
	// If the cache is already valid we just reuse it.
	if (patchId < m_entries.size())
	{
		if (/*m_updateCount >= c_maxUpdatePerFrame ||*/ (m_entries[patchId].lod == surfaceLod && m_entries[patchId].tile.dim > 0))
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

	render::Shader* shader = terrain->getSurfaceShader();
	if (!shader)
		return;

	render::ISimpleTexture* heightMap = terrain->getHeightMap();
	render::ISimpleTexture* colorMap = terrain->getColorMap();
	render::ISimpleTexture* splatMap = terrain->getSplatMap();

	shader->setCombination(m_handleColorEnable, colorMap != 0);

	if (!shader->getCurrentProgram())
		return;

	TerrainSurfaceRenderBlock* renderBlock = renderContext->alloc< TerrainSurfaceRenderBlock >("Terrain surface");

	renderBlock->screenRenderer = m_screenRenderer;
	renderBlock->distance = 0.0f;
	renderBlock->program = shader->getCurrentProgram();
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();

	renderBlock->programParams->beginParameters(renderContext);
	renderBlock->programParams->setTextureParameter(m_handleHeightfield, heightMap);
	renderBlock->programParams->setTextureParameter(m_handleColorMap, colorMap);
	renderBlock->programParams->setTextureParameter(m_handleSplatMap, splatMap);
	renderBlock->programParams->setVectorParameter(m_handleWorldOrigin, worldOriginM);
	renderBlock->programParams->setVectorParameter(m_handleWorldExtent, worldExtentM);
	renderBlock->programParams->setVectorParameter(m_handlePatchOrigin, patchOriginM);
	renderBlock->programParams->setVectorParameter(m_handlePatchExtent, patchExtentM);

	Vector4 textureOffset(
		-1.0f + 2.0f * tile.x / 4096.0f,
		 1.0f - 2.0f * tile.y / 4096.0f,
		 2.0f * tile.dim / 4096.0f,
		-2.0f * tile.dim / 4096.0f
	);
	renderBlock->programParams->setVectorParameter(m_handleTextureOffset, textureOffset);
	renderBlock->programParams->endParameters(renderContext);

	renderBlock->renderTargetSet = m_pool;
	renderBlock->clear = m_clearCache;

	if (outRenderBlock)
	{
		TerrainSurfaceRenderBlock* renderBlockChain = static_cast< TerrainSurfaceRenderBlock* >(outRenderBlock);

		renderBlockChain->renderTargetSet = 0;
		renderBlockChain->clear = false;

		renderBlock->next = renderBlockChain;
	}
	
	m_clearCache = false;

	// Update cache entry.
	m_entries[patchId].lod = surfaceLod;
	m_entries[patchId].tile = tile;

	outTextureOffset = offsetFromTile(tile, m_size);
	outRenderBlock = renderBlock;
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
