#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/TerrainSurface.h"
#include "World/WorldRenderView.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"

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

	TerrainSurfaceRenderBlock()
	:	screenRenderer(0)
	,	renderTargetSet(0)
	{
	}

	virtual void render(render::IRenderView* renderView) const
	{
		if (shaderParams)
			shaderParams->fixup(shader);

		renderView->begin(renderTargetSet, 0, false);

		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		renderView->clear(render::CfColor, clearColor, 0.0f, 0);

		screenRenderer->draw(renderView, shader);

		renderView->end();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainSurfaceCache", TerrainSurfaceCache, Object)

TerrainSurfaceCache::TerrainSurfaceCache()
:	m_handleHeightfield(render::getParameterHandle(L"Heightfield"))
,	m_handleHeightfieldSize(render::getParameterHandle(L"HeightfieldSize"))
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

	m_screenRenderer = gc_new< render::ScreenRenderer >();
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

void TerrainSurfaceCache::get(
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
)
{
	// If the cache is already valid we just reuse it.
	if (patchId < m_entries.size())
	{
		if (m_entries[patchId].lod == surfaceLod && m_entries[patchId].renderTargetSet)
		{
			if (m_entries[patchId].renderTargetSet->isContentValid())
			{
				outRenderBlock = 0;
				outTexture = m_entries[patchId].renderTargetSet->getColorTexture(0);
				return;
			}
		}
	
		// Release cache as it's no longer valid.
		flush(patchId);
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

	resource::Proxy< render::Shader >& shader = surface->getLayers().front();
	if (!m_resourceManager->bind(shader))
		return;

	// Create render block.
	TerrainSurfaceRenderBlock* renderBlock = renderContext->alloc< TerrainSurfaceRenderBlock >();

	renderBlock->screenRenderer = m_screenRenderer;
	renderBlock->renderTargetSet = renderTargetSet;

	renderBlock->type = render::RbtOpaque;
	renderBlock->distance = 0.0f;
	renderBlock->shader = shader;
	renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();

	renderBlock->shaderParams->beginParameters(renderContext);

	renderBlock->shaderParams->setSamplerTexture(m_handleHeightfield, heightfieldTexture);
	renderBlock->shaderParams->setFloatParameter(m_handleHeightfieldSize, float(heightfieldTexture->getWidth()));
	renderBlock->shaderParams->setVectorParameter(m_handleWorldOrigin, worldOrigin);
	renderBlock->shaderParams->setVectorParameter(m_handleWorldExtent, worldExtent);
	renderBlock->shaderParams->setVectorParameter(m_handlePatchOrigin, patchOrigin);
	renderBlock->shaderParams->setVectorParameter(m_handlePatchExtent, patchExtent);

	worldRenderView->setShaderParameters(renderBlock->shaderParams);

	renderBlock->shaderParams->endParameters(renderContext);

	// Update cache entry.
	m_entries[patchId].lod = surfaceLod;
	m_entries[patchId].renderTargetSet = renderTargetSet;

	outRenderBlock = renderBlock;
	outTexture = renderTargetSet->getColorTexture(0);
}

Ref< render::RenderTargetSet > TerrainSurfaceCache::allocateTarget(uint32_t lod)
{
	render::RenderTargetSetCreateDesc desc;

	desc.count = 1;
	desc.width = c_cacheSurfaceSize[lod];
	desc.height = c_cacheSurfaceSize[lod];
	desc.multiSample = 0;
	desc.depthStencil = false;
	desc.targets[0].format = render::TfR8G8B8A8;

	return m_renderSystem->createRenderTargetSet(desc);
}

	}
}
