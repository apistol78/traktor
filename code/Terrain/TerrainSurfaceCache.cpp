#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
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

const render::Handle c_handleColorEnable(L"ColorEnable");
const render::Handle c_handleHeightfield(L"Heightfield");
const render::Handle c_handleColorMap(L"ColorMap");
const render::Handle c_handleSplatMap(L"SplatMap");
const render::Handle c_handleWorldOrigin(L"WorldOrigin");
const render::Handle c_handleWorldExtent(L"WorldExtent");
const render::Handle c_handlePatchOrigin(L"PatchOrigin");
const render::Handle c_handlePatchExtent(L"PatchExtent");
const render::Handle c_handleTextureOffset(L"TextureOffset");

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
,	m_size(0)
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

	m_pool = renderSystem->createRenderTargetSet(desc, nullptr, T_FILE_LINE_W);
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

	m_base = renderSystem->createRenderTargetSet(desc, nullptr, T_FILE_LINE_W);
	if (!m_base)
		return false;

	m_haveBase = false;
	m_clearCache = true;
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
	m_haveBase = false;
}

void TerrainSurfaceCache::setupBaseColor(
	render::RenderGraph& renderGraph,
	Terrain* terrain,
	const Vector4& worldOrigin,
	const Vector4& worldExtent
)
{
	if (m_haveBase)
		return;

	auto baseTargetSetId = renderGraph.addTargetSet(m_base);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Terrain surface base");
	
	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	rp->setOutput(baseTargetSetId, clear);

	rp->addBuild([=](const render::RenderGraph&, render::RenderContext* renderContext) {
		const static Vector4 c_textureOffset(-1.0f, 1.0f, 2.0f, -2.0f);

		render::Shader* shader = terrain->getSurfaceShader();
		if (!shader)
			return;

		render::ISimpleTexture* heightMap = terrain->getHeightMap();
		render::ISimpleTexture* colorMap = terrain->getColorMap();
		render::ISimpleTexture* splatMap = terrain->getSplatMap();

		render::Shader::Permutation perm;
		shader->setCombination(c_handleColorEnable, colorMap != nullptr, perm);

		auto sp = shader->getProgram(perm);
		if (!sp)
			return;

		auto rb = renderContext->alloc< TerrainSurfaceRenderBlock >(L"Terrain surface, base");
		rb->screenRenderer = m_screenRenderer;
		rb->distance = 0.0f;
		rb->program = sp.program;
		rb->programParams = renderContext->alloc< render::ProgramParameters >();
		rb->programParams->beginParameters(renderContext);
		rb->programParams->setTextureParameter(c_handleHeightfield, heightMap);
		rb->programParams->setTextureParameter(c_handleColorMap, colorMap);
		rb->programParams->setTextureParameter(c_handleSplatMap, splatMap);
		rb->programParams->setVectorParameter(c_handleWorldOrigin, worldOrigin);
		rb->programParams->setVectorParameter(c_handleWorldExtent, worldExtent);
		rb->programParams->setVectorParameter(c_handlePatchOrigin, worldOrigin);
		rb->programParams->setVectorParameter(c_handlePatchExtent, worldExtent);
		rb->programParams->setVectorParameter(c_handleTextureOffset, c_textureOffset);
		rb->programParams->endParameters(renderContext);
		renderContext->enqueue(rb);
	});
	renderGraph.addPass(rp);

	m_haveBase = true;

	// Since this method is called after patches has been
	// setup it's safe to clear reference to patch pass here.
	m_surfacePass = nullptr;
}

void TerrainSurfaceCache::setupPatch(
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
)
{
	// If the cache is already valid we just reuse it.
	if (patchId < m_entries.size())
	{
		if (m_entries[patchId].lod == surfaceLod && m_entries[patchId].tile.dim > 0)
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

	// Allocate pass for this frame.
	if (!m_surfacePass)
	{
		auto poolTargetSetId = renderGraph.addTargetSet(m_pool);
		m_surfacePass = new render::RenderPass(L"Terrain surface");
		if (!m_clearCache)
			m_surfacePass->setOutput(poolTargetSetId);
		else
		{
			render::Clear clear;
			clear.mask = render::CfColor;
			clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
			m_surfacePass->setOutput(poolTargetSetId, clear);
			m_clearCache = false;
		}
		renderGraph.addPass(m_surfacePass);
	}

	// Add build step to pass.
	m_surfacePass->addBuild([=](const render::RenderGraph&, render::RenderContext* renderContext) {
		render::Shader* shader = terrain->getSurfaceShader();
		if (!shader)
			return;

		render::ISimpleTexture* heightMap = terrain->getHeightMap();
		render::ISimpleTexture* colorMap = terrain->getColorMap();
		render::ISimpleTexture* splatMap = terrain->getSplatMap();

		render::Shader::Permutation perm;
		shader->setCombination(c_handleColorEnable, colorMap != nullptr, perm);

		auto sp = shader->getProgram(perm);
		if (!sp)
			return;

		auto rb = renderContext->alloc< TerrainSurfaceRenderBlock >(L"Terrain surface");
		rb->screenRenderer = m_screenRenderer;
		rb->distance = 0.0f;
		rb->program = sp.program;
		rb->programParams = renderContext->alloc< render::ProgramParameters >();
		rb->programParams->beginParameters(renderContext);
		rb->programParams->setTextureParameter(c_handleHeightfield, heightMap);
		rb->programParams->setTextureParameter(c_handleColorMap, colorMap);
		rb->programParams->setTextureParameter(c_handleSplatMap, splatMap);
		rb->programParams->setVectorParameter(c_handleWorldOrigin, worldOriginM);
		rb->programParams->setVectorParameter(c_handleWorldExtent, worldExtentM);
		rb->programParams->setVectorParameter(c_handlePatchOrigin, patchOriginM);
		rb->programParams->setVectorParameter(c_handlePatchExtent, patchExtentM);
		rb->programParams->setVectorParameter(c_handleTextureOffset, textureOffset);
		rb->programParams->endParameters(renderContext);
		renderContext->enqueue(rb);
	});

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
