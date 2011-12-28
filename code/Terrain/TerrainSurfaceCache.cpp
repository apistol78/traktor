#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/TerrainSurface.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

struct TerrainSurfaceRenderBlock : public render::RenderBlock
{
	render::ScreenRenderer* screenRenderer;
	render::RenderTargetSet* renderTargetSet;
	TerrainSurfaceRenderBlock* next;
	bool top;

	TerrainSurfaceRenderBlock()
	:	screenRenderer(0)
	,	renderTargetSet(0)
	,	next(0)
	,	top(false)
	{
	}

	virtual void render(render::IRenderView* renderView, const render::ProgramParameters* globalParameters) const
	{
		if (programParams)
			programParams->fixup(program);
		if (globalParameters)
			globalParameters->fixup(program);

		if (top)
			renderView->begin(renderTargetSet, 0);

		screenRenderer->draw(renderView, program);

		if (next)
			next->render(renderView, globalParameters);

		if (top)
			renderView->end();

		T_SAFE_RELEASE(renderTargetSet);
	}
};

Vector4 offsetFromTile(const TerrainSurfaceAlloc::Tile& tile)
{
	return Vector4(
		tile.x / 4096.0f,
		tile.y / 4096.0f,
		tile.dim / 4096.0f,
		tile.dim / 4096.0f
	);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainSurfaceCache", TerrainSurfaceCache, Object)

TerrainSurfaceCache::TerrainSurfaceCache()
:	m_updateAllowedCount(0)
,	m_handleHeightfield(render::getParameterHandle(L"Heightfield"))
,	m_handleHeightfieldSize(render::getParameterHandle(L"HeightfieldSize"))
,	m_handleMaterialMask(render::getParameterHandle(L"MaterialMask"))
,	m_handleMaterialMaskSize(render::getParameterHandle(L"MaterialMaskSize"))
,	m_handleMaterial(render::getParameterHandle(L"Material"))
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

bool TerrainSurfaceCache::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
{
	m_resourceManager = resourceManager;
	m_renderSystem = renderSystem;

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	render::RenderTargetSetCreateDesc desc;

	desc.count = 1;
	desc.width = 4096;
	desc.height = 4096;
	desc.multiSample = 0;
	desc.createDepthStencil = false;
	desc.usingPrimaryDepthStencil = false;
	desc.targets[0].format = render::TfR8G8B8A8;

	m_pool = renderSystem->createRenderTargetSet(desc);
	if (!m_pool)
		return false;

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

void TerrainSurfaceCache::begin()
{
	m_updateAllowedCount = 2;
}

void TerrainSurfaceCache::get(
	render::RenderContext* renderContext,
	TerrainSurface* surface,
	render::ISimpleTexture* heightfieldTexture,
	render::ISimpleTexture* materialMaskTexture,
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
		if (m_updateAllowedCount > 0)
		{
			if (
				m_entries[patchId].lod == surfaceLod &&
				m_entries[patchId].tile.dim > 0 &&
				m_pool->isContentValid()
			)
			{
				outRenderBlock = 0;
				outTextureOffset = offsetFromTile(m_entries[patchId].tile);
				return;
			}
		}
		else
		{
			// We've already consumed our budget of updates for this frame; only
			// update if absolutely necessary.
			if (m_pool->isContentValid())
			{
				outRenderBlock = 0;
				outTextureOffset = offsetFromTile(m_entries[patchId].tile);
				return;
			}
		}
	
		// Release cache as it's no longer valid.
		flush(patchId);
		m_updateAllowedCount--;
	}
	else
	{
		// Patch hasn't been cached before, allocate a new entry.
		m_entries.resize(patchId + 1);
	}

	// Allocate tile for this patch.
	TerrainSurfaceAlloc::Tile tile;
	if (!m_alloc.alloc(surfaceLod, tile))
	{
		outRenderBlock = 0;
		outTextureOffset = Vector4::zero();
		return;
	}

	// Composite surface.
	TerrainSurfaceRenderBlock* renderBlockChain = 0;

	std::vector< resource::Proxy< render::Shader > >& layers = surface->getLayers();
	for (size_t i = 0; i < layers.size(); ++i)
	{
		resource::Proxy< render::Shader >& shader = layers[i];
		if (!shader.validate())
			continue;

		TerrainSurfaceRenderBlock* renderBlock = renderContext->alloc< TerrainSurfaceRenderBlock >("Terrain surface");

		renderBlock->screenRenderer = m_screenRenderer;
		renderBlock->renderTargetSet = m_pool;

		renderBlock->distance = 0.0f;
		renderBlock->program = shader->getCurrentProgram();
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();

		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setTextureParameter(m_handleHeightfield, heightfieldTexture);
		renderBlock->programParams->setFloatParameter(m_handleHeightfieldSize, float(heightfieldTexture->getWidth()));
		renderBlock->programParams->setTextureParameter(m_handleMaterialMask, materialMaskTexture);
		renderBlock->programParams->setFloatParameter(m_handleMaterialMaskSize, float(materialMaskTexture->getWidth()));
		renderBlock->programParams->setFloatParameter(m_handleMaterial, float(i));
		renderBlock->programParams->setVectorParameter(m_handleWorldOrigin, worldOrigin);
		renderBlock->programParams->setVectorParameter(m_handleWorldExtent, worldExtent);
		renderBlock->programParams->setVectorParameter(m_handlePatchOrigin, patchOrigin);
		renderBlock->programParams->setVectorParameter(m_handlePatchExtent, patchExtent);

		Vector4 textureOffset(
			2.0f * tile.x / 4096.0f - 1.0f,
			1.0f - 2.0f * tile.y / 4096.0f,
			2.0f * tile.dim / 4096.0f,
			-2.0f * tile.dim / 4096.0f
		);
		renderBlock->programParams->setVectorParameter(m_handleTextureOffset, textureOffset);
		renderBlock->programParams->endParameters(renderContext);

		T_SAFE_ADDREF(renderBlock->renderTargetSet);

		if (!renderBlockChain)
		{
			renderBlock->top = true;
			renderBlockChain = renderBlock;
			outRenderBlock = renderBlock;
		}
		else
		{
			renderBlock->top = false;
			renderBlockChain->next = renderBlock;
			renderBlockChain = renderBlock;
		}
	}

	// Update cache entry.
	m_entries[patchId].lod = surfaceLod;
	m_entries[patchId].tile = tile;

	outTextureOffset = offsetFromTile(tile);
}

render::ITexture* TerrainSurfaceCache::getVirtualTexture() const
{
	return m_pool->getColorTexture(0);
}

	}
}
