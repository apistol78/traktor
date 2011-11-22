#include "Core/Log/Log.h"
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

#if !defined(_XBOX)
const uint32_t c_cacheSurfaceSize[] = { 1024, 512, 256, 128 };
#else
const uint32_t c_cacheSurfaceSize[] = { 512, 256, 128, 64 };
#endif

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
		{
			renderView->begin(renderTargetSet, 0);

			const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			renderView->clear(render::CfColor, clearColor, 0.0f, 0);
		}

		screenRenderer->draw(renderView, program);

		if (next)
			next->render(renderView, globalParameters);

		if (top)
			renderView->end();

		T_SAFE_RELEASE(renderTargetSet);
	}
};

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

	return true;
}

void TerrainSurfaceCache::destroy()
{
	if (m_entries.empty())
		return;

	flush();

	for (uint32_t i = 0; i < sizeof_array(m_cache); ++i)
	{
		for (RefArray< render::RenderTargetSet >::iterator j = m_cache[i].begin(); j != m_cache[i].end(); ++j)
		{
			if (*j)
				(*j)->destroy();
		}
		m_cache[i].resize(0);
	}

	if (m_screenRenderer)
	{
		m_screenRenderer->destroy();
		m_screenRenderer = 0;
	}
}

void TerrainSurfaceCache::flush(uint32_t patchId)
{
	if (patchId >= m_entries.size())
		return;

	Entry& entry = m_entries[patchId];
	if (entry.renderTargetSet)
	{
		m_cache[entry.lod].push_back(entry.renderTargetSet);
		entry.renderTargetSet = 0;
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
	Ref< render::ISimpleTexture >& outTexture
)
{
	// If the cache is already valid we just reuse it.
	if (patchId < m_entries.size())
	{
		if (m_updateAllowedCount > 0)
		{
			if (
				m_entries[patchId].lod == surfaceLod &&
				m_entries[patchId].renderTargetSet &&
				m_entries[patchId].renderTargetSet->isContentValid()
			)
			{
				outRenderBlock = 0;
				outTexture = m_entries[patchId].renderTargetSet->getColorTexture(0);
				return;
			}
		}
		else
		{
			// We've already consumed our budget of updates for this frame; only
			// update if absolutely necessary.
			if (m_entries[patchId].renderTargetSet && m_entries[patchId].renderTargetSet->isContentValid())
			{
				outRenderBlock = 0;
				outTexture = m_entries[patchId].renderTargetSet->getColorTexture(0);
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

	// Allocate target for this patch.
	Ref< render::RenderTargetSet > renderTargetSet;

	if (!m_cache[surfaceLod].empty())
	{
		renderTargetSet = m_cache[surfaceLod].back();
		m_cache[surfaceLod].pop_back();
	}
	else
	{
		if (!(renderTargetSet = allocateTarget(surfaceLod)))
		{
			outRenderBlock = 0;
			outTexture = 0;
			return;
		}
	}

	// Composite surface.
	TerrainSurfaceRenderBlock* renderBlockChain = 0;

	std::vector< resource::Proxy< render::Shader > >& layers = surface->getLayers();
	for (size_t i = 0; i < layers.size(); ++i)
	{
		resource::Proxy< render::Shader >& shader = layers[i];
		if (!shader.validate())
			return;

		TerrainSurfaceRenderBlock* renderBlock = renderContext->alloc< TerrainSurfaceRenderBlock >("Terrain surface");

		renderBlock->screenRenderer = m_screenRenderer;
		renderBlock->renderTargetSet = renderTargetSet;

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
	m_entries[patchId].renderTargetSet = renderTargetSet;

	outTexture = renderTargetSet->getColorTexture(0);
}

Ref< render::RenderTargetSet > TerrainSurfaceCache::allocateTarget(uint32_t lod)
{
	render::RenderTargetSetCreateDesc desc;

	desc.count = 1;
	desc.width = c_cacheSurfaceSize[lod];
	desc.height = c_cacheSurfaceSize[lod];
	desc.multiSample = 0;
	desc.createDepthStencil = false;
	desc.usingPrimaryDepthStencil = false;
	desc.targets[0].format = render::TfR8G8B8A8;

	return m_renderSystem->createRenderTargetSet(desc);
}

	}
}
