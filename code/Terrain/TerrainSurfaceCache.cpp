/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

namespace traktor::terrain
{
	namespace
	{

const render::Handle c_handleTerrain_ColorEnable(L"Terrain_ColorEnable");
const render::Handle c_handleTerrain_Heightfield(L"Terrain_Heightfield");
const render::Handle c_handleTerrain_ColorMap(L"Terrain_ColorMap");
const render::Handle c_handleTerrain_SplatMap(L"Terrain_SplatMap");
const render::Handle c_handleTerrain_WorldOrigin(L"Terrain_WorldOrigin");
const render::Handle c_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle c_handleTerrain_PatchOrigin(L"Terrain_PatchOrigin");
const render::Handle c_handleTerrain_PatchExtent(L"Terrain_PatchExtent");
const render::Handle c_handleTerrain_TextureOffset(L"Terrain_TextureOffset");

const uint32_t c_maxUpdatePerFrame = 1;
const int32_t c_margin = 2;

Vector4 offsetFromTile(const TerrainSurfaceAlloc& alloc, const TerrainSurfaceAlloc::Tile& tile)
{
	float virtualSize = (float)alloc.getVirtualSize();
	return Vector4(
		(tile.x + c_margin) / virtualSize,
		(tile.y + c_margin) / virtualSize,
		(tile.dim - 2.0f * c_margin) / virtualSize,
		(tile.dim - 2.0f * c_margin) / virtualSize
	);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainSurfaceCache", TerrainSurfaceCache, Object)

TerrainSurfaceCache::TerrainSurfaceCache()
:	m_alloc(1024)
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

	m_alloc = TerrainSurfaceAlloc(size);

	// Create virtual terrain texture.
	{
		render::RenderTargetSetCreateDesc desc = {};
		desc.count = 2;
		desc.width = size;
		desc.height = size;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = false;
		desc.targets[0].format = render::TfR8G8B8A8;
		desc.targets[1].format = render::TfR11G11B10F;
		m_virtualTexture = renderSystem->createRenderTargetSet(desc, nullptr, T_FILE_LINE_W);
		if (!m_virtualTexture)
			return false;
	}

	// Create base texture, a very low resolution copy
	// of the entire terrain surface.
	{
		render::RenderTargetSetCreateDesc desc = {};
		desc.count = 1;
		desc.width = 512;
		desc.height = 512;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = false;
		desc.targets[0].format = render::TfR8G8B8A8;
		m_base = renderSystem->createRenderTargetSet(desc, nullptr, T_FILE_LINE_W);
		if (!m_base)
			return false;
	}

	m_haveBase = false;
	m_clearCache = true;
	return true;
}

void TerrainSurfaceCache::destroy()
{
	if (m_entries.empty())
		return;

	flush();

	safeDestroy(m_virtualTexture);
	safeDestroy(m_base);
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
	if (!m_haveBase)
	{
		auto baseTargetSetId = renderGraph.addTargetSet(L"Terrain base", m_base);

		Ref< render::RenderPass > rp = new render::RenderPass(L"Terrain surface base");
	
		render::Clear clear;
		clear.mask = render::CfColor;
		clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
		rp->setOutput(baseTargetSetId, clear, render::TfNone, render::TfColor);

		rp->addBuild([=](const render::RenderGraph&, render::RenderContext* renderContext) {
			const static Vector4 c_textureOffset(-1.0f, 1.0f, 2.0f, -2.0f);

			render::Shader* shader = terrain->getSurfaceShader();
			if (!shader)
				return;

			render::ISimpleTexture* heightMap = terrain->getHeightMap();
			render::ISimpleTexture* colorMap = terrain->getColorMap();
			render::ISimpleTexture* splatMap = terrain->getSplatMap();

			render::Shader::Permutation perm;
			shader->setCombination(c_handleTerrain_ColorEnable, colorMap != nullptr, perm);

			auto sp = shader->getProgram(perm);
			if (!sp)
				return;

			auto programParams = renderContext->alloc< render::ProgramParameters >();
			programParams->beginParameters(renderContext);
			programParams->setTextureParameter(c_handleTerrain_Heightfield, heightMap);
			programParams->setTextureParameter(c_handleTerrain_ColorMap, colorMap);
			programParams->setTextureParameter(c_handleTerrain_SplatMap, splatMap);
			programParams->setVectorParameter(c_handleTerrain_WorldOrigin, worldOrigin);
			programParams->setVectorParameter(c_handleTerrain_WorldExtent, worldExtent);
			programParams->setVectorParameter(c_handleTerrain_PatchOrigin, worldOrigin);
			programParams->setVectorParameter(c_handleTerrain_PatchExtent, worldExtent);
			programParams->setVectorParameter(c_handleTerrain_TextureOffset, c_textureOffset);
			programParams->endParameters(renderContext);

			m_screenRenderer->draw(renderContext, sp.program, programParams);
		});
		renderGraph.addPass(rp);

		m_haveBase = true;
	}

	m_updateCount = 0;
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
			outTextureOffset = offsetFromTile(m_alloc, m_entries[patchId].tile);
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

	if (m_updateCount++ >= c_maxUpdatePerFrame)
	{
		outTextureOffset = Vector4::zero();
		return;
	}

	// Allocate tile for this patch; first try to allocate proper size
	// then fall back on smaller and smaller tiles.
	TerrainSurfaceAlloc::Tile tile = {};
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

	const Vector4 worldOriginM = worldOrigin;
	const Vector4 worldExtentM = worldExtent;

	// Number of patch units per texel.
	const float f = (float)m_alloc.getVirtualSize() / tile.dim;
	const float dpx = f * patchExtent.x() / m_alloc.getVirtualSize();
	const float dpz = f * patchExtent.z() / m_alloc.getVirtualSize();

	Vector4 patchOriginM = patchOrigin;
	Vector4 patchExtentM = patchExtent;

	patchOriginM -= Vector4(dpx, 0.0f, dpz, 0.0f) * Scalar(c_margin);
	patchExtentM += Vector4(dpx, 0.0f, dpz, 0.0f) * Scalar(2.0f * c_margin);

	// Bake terrain surface patch into virtual texture.
	{
		auto virtualTargetSetId = renderGraph.addTargetSet(L"Terrain surface", m_virtualTexture);

		Ref< render::RenderPass > updatePass = new render::RenderPass(L"Terrain surface update");
		updatePass->setOutput(virtualTargetSetId, render::TfColor, render::TfColor);
		updatePass->addBuild([=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
			render::Shader* shader = terrain->getSurfaceShader();
			if (!shader)
				return;

			render::ISimpleTexture* heightMap = terrain->getHeightMap();
			render::ISimpleTexture* colorMap = terrain->getColorMap();
			render::ISimpleTexture* splatMap = terrain->getSplatMap();

			render::Shader::Permutation perm;
			shader->setCombination(c_handleTerrain_ColorEnable, colorMap != nullptr, perm);

			auto sp = shader->getProgram(perm);
			if (!sp)
				return;

			auto programParams = renderContext->alloc< render::ProgramParameters >();
			programParams->beginParameters(renderContext);
			programParams->setTextureParameter(c_handleTerrain_Heightfield, heightMap);
			programParams->setTextureParameter(c_handleTerrain_ColorMap, colorMap);
			programParams->setTextureParameter(c_handleTerrain_SplatMap, splatMap);
			programParams->setVectorParameter(c_handleTerrain_WorldOrigin, worldOriginM);
			programParams->setVectorParameter(c_handleTerrain_WorldExtent, worldExtentM);
			programParams->setVectorParameter(c_handleTerrain_PatchOrigin, patchOriginM);
			programParams->setVectorParameter(c_handleTerrain_PatchExtent, patchExtentM);
			programParams->endParameters(renderContext);

			auto svrb = renderContext->alloc< render::SetViewportRenderBlock >();
			svrb->viewport = render::Viewport(
				tile.x,
				tile.y,
				tile.dim,
				tile.dim,
				0.0f,
				1.0f
			);

			m_screenRenderer->draw(renderContext, sp.program, programParams);
		});
		renderGraph.addPass(updatePass);
	}

	// Update cache entry.
	m_entries[patchId].lod = surfaceLod;
	m_entries[patchId].tile = tile;

	outTextureOffset = offsetFromTile(m_alloc, tile);
}

render::ISimpleTexture* TerrainSurfaceCache::getVirtualAlbedo() const
{
	return m_virtualTexture->getColorTexture(0);
}

render::ISimpleTexture* TerrainSurfaceCache::getVirtualNormals() const
{
	return m_virtualTexture->getColorTexture(1);
}

render::ISimpleTexture* TerrainSurfaceCache::getBaseTexture() const
{
	return m_base->getColorTexture(0);
}

}
