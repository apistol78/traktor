/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/TerrainSurfaceCache.h"

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "World/IWorldRenderPass.h"

namespace traktor::terrain
{
namespace
{

// Techniques
const render::Handle c_handleTerrain_SurfaceBase(L"Terrain_SurfaceBase");

const render::Handle c_handleTerrain_ColorEnable(L"Terrain_ColorEnable");
const render::Handle c_handleTerrain_Heightfield(L"Terrain_Heightfield");
const render::Handle c_handleTerrain_Normals(L"Terrain_Normals");
const render::Handle c_handleTerrain_ColorMap(L"Terrain_ColorMap");
const render::Handle c_handleTerrain_SplatMap(L"Terrain_SplatMap");
const render::Handle c_handleTerrain_WorldOrigin(L"Terrain_WorldOrigin");
const render::Handle c_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle c_handleTerrain_PatchOrigin(L"Terrain_PatchOrigin");
const render::Handle c_handleTerrain_PatchExtent(L"Terrain_PatchExtent");
const render::Handle c_handleTerrain_TextureOffset(L"Terrain_TextureOffset");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainSurfaceCache", TerrainSurfaceCache, Object)

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

	// Create base texture, a low resolution copy
	// of the entire terrain surface.
	{
		render::RenderTargetSetCreateDesc desc = {};
		desc.count = 1;
		desc.width = 2048;
		desc.height = 2048;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = false;
		desc.targets[0].format = render::TfR8G8B8A8;
		m_base = renderSystem->createRenderTargetSet(desc, nullptr, T_FILE_LINE_W);
		if (!m_base)
			return false;
	}

	m_haveBase = false;
	return true;
}

void TerrainSurfaceCache::destroy()
{
	safeDestroy(m_base);
	safeDestroy(m_screenRenderer);
}

void TerrainSurfaceCache::flush()
{
	m_haveBase = false;
}

bool TerrainSurfaceCache::setupBaseColor(
	render::RenderGraph& renderGraph,
	Terrain* terrain,
	const Vector4& worldOrigin,
	const Vector4& worldExtent)
{
	bool updatedBase = false;

	if (!m_haveBase)
	{
		auto baseTargetSetId = renderGraph.addExplicitTargetSet(L"Terrain base", m_base);

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

			render::ITexture* heightMap = terrain->getHeightMap();
			render::ITexture* normalMap = terrain->getNormalMap();
			render::ITexture* colorMap = terrain->getColorMap();
			render::ITexture* splatMap = terrain->getSplatMap();

			render::Shader::Permutation perm(c_handleTerrain_SurfaceBase);
			shader->setCombination(c_handleTerrain_ColorEnable, colorMap != nullptr, perm);

			auto sp = shader->getProgram(perm);
			if (!sp)
				return;

			auto programParams = renderContext->alloc< render::ProgramParameters >();
			programParams->beginParameters(renderContext);
			programParams->setTextureParameter(c_handleTerrain_Heightfield, heightMap);
			programParams->setTextureParameter(c_handleTerrain_Normals, normalMap);
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

		updatedBase = true;
		m_haveBase = true;
	}

	m_updateCount = 0;
	return updatedBase;
}

render::ITexture* TerrainSurfaceCache::getBaseTexture() const
{
	return m_base->getColorTexture(0);
}

}
