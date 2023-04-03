/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/IRenderSystem.h"
#include "Render/ScreenRenderer.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/IEntityRenderer.h"
#include "World/WorldGatherContext.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Entity/VolumetricFogComponent.h"
#include "World/Shared/WorldRendererShared.h"
#include "World/Shared/Passes/AmbientOcclusionPass.h"
#include "World/Shared/Passes/GBufferPass.h"
#include "World/Shared/Passes/LightClusterPass.h"
#include "World/Shared/Passes/PostProcessPass.h"
#include "World/Shared/Passes/ReflectionsPass.h"
#include "World/Shared/Passes/VelocityPass.h"

namespace traktor::world
{
	namespace
	{

Ref< render::ITexture > create1x1Texture(render::IRenderSystem* renderSystem, uint32_t value)
{
	render::SimpleTextureCreateDesc stcd = {};
	stcd.width = 1;
	stcd.height = 1;
	stcd.mipCount = 1;
	stcd.format = render::TfR8G8B8A8;
	stcd.sRGB = false;
	stcd.immutable = true;
	stcd.initialData[0].data = &value;
	stcd.initialData[0].pitch = 4;
	return renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
}

Ref< render::ITexture > createCubeTexture(render::IRenderSystem* renderSystem, uint32_t value)
{
	render::CubeTextureCreateDesc ctcd = {};
	ctcd.side = 1;
	ctcd.mipCount = 1;
	ctcd.format = render::TfR8G8B8A8;
	ctcd.sRGB = false;
	ctcd.immutable = true;
	ctcd.initialData[0].data = &value;
	ctcd.initialData[0].pitch = 4;
	ctcd.initialData[1].data = &value;
	ctcd.initialData[1].pitch = 4;
	ctcd.initialData[2].data = &value;
	ctcd.initialData[2].pitch = 4;
	ctcd.initialData[3].data = &value;
	ctcd.initialData[3].pitch = 4;
	ctcd.initialData[4].data = &value;
	ctcd.initialData[4].pitch = 4;
	ctcd.initialData[5].data = &value;
	ctcd.initialData[0].pitch = 4;
	return renderSystem->createCubeTexture(ctcd, T_FILE_LINE_W);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRendererShared", WorldRendererShared, IWorldRenderer)

bool WorldRendererShared::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	m_entityRenderers = desc.entityRenderers;

	// Store settings.
	m_settings = *desc.worldRenderSettings;
	m_shadowsQuality = desc.quality.shadows;
	m_sharedDepthStencil = desc.sharedDepthStencil;

	m_imageGraphContext = new render::ImageGraphContext();
	
	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Create default value textures.
	m_blackTexture = create1x1Texture(renderSystem, 0x00000000);
	m_whiteTexture = create1x1Texture(renderSystem, 0xffffffff);
	m_blackCubeTexture = createCubeTexture(renderSystem, 0x00000000);

	// Create shared passes.
	m_lightClusterPass = new LightClusterPass(m_settings);
	if (!m_lightClusterPass->create(renderSystem))
		return false;

	m_gbufferPass = new GBufferPass(m_settings, m_entityRenderers, m_sharedDepthStencil);

	m_velocityPass = new VelocityPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_velocityPass->create(resourceManager, renderSystem, desc))
		return false;

	m_ambientOcclusionPass = new AmbientOcclusionPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_ambientOcclusionPass->create(resourceManager, renderSystem, desc))
		return false;

	m_reflectionsPass = new ReflectionsPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_reflectionsPass->create(resourceManager, renderSystem, desc))
		return false;

	m_postProcessPass = new PostProcessPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_postProcessPass->create(resourceManager, renderSystem, desc))
		return false;

	return true;
}

void WorldRendererShared::destroy()
{
	safeDestroy(m_screenRenderer);
	safeDestroy(m_blackTexture);
	safeDestroy(m_whiteTexture);

	m_postProcessPass = nullptr;
	m_reflectionsPass = nullptr;
	m_ambientOcclusionPass = nullptr;
	m_velocityPass = nullptr;
	m_gbufferPass = nullptr;

	safeDestroy(m_lightClusterPass);
}

render::ImageGraphContext* WorldRendererShared::getImageGraphContext() const
{
	return m_imageGraphContext;
}

void WorldRendererShared::gather(Entity* rootEntity)
{
	T_PROFILER_SCOPE(L"WorldRendererShared::gather");
	StaticVector< const LightComponent*, LightClusterPass::c_maxLightCount > lights;

	m_gatheredView.renderables.resize(0);
	m_gatheredView.lights.resize(0);
	m_gatheredView.probes.resize(0);
	m_gatheredView.fogs.resize(0);

	WorldGatherContext(m_entityRenderers, rootEntity, [&](IEntityRenderer* entityRenderer, Object* renderable) {

		// Filter out components used to setup frame's lighting etc.
		if (auto lightComponent = dynamic_type_cast< const LightComponent* >(renderable))
		{
			if (!lights.full())
				lights.push_back(lightComponent);
		}
		else if (auto probeComponent = dynamic_type_cast< const ProbeComponent* >(renderable))
			m_gatheredView.probes.push_back(probeComponent);
		else if (auto volumetricFogComponent = dynamic_type_cast< const VolumetricFogComponent* >(renderable))
			m_gatheredView.fogs.push_back(volumetricFogComponent);

		m_gatheredView.renderables.push_back({ entityRenderer, renderable });

	}).gather(rootEntity);

	// Arrange lights.
	{
		const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);

		m_gatheredView.lights.resize(4);
		m_gatheredView.lights[0] = nullptr;
		m_gatheredView.lights[1] = nullptr;
		m_gatheredView.lights[2] = nullptr;
		m_gatheredView.lights[3] = nullptr;

		// Find cascade shadow light; must be first.
		if (shadowsEnable)
		{
			for (int32_t i = 0; i < (int32_t)lights.size(); ++i)
			{
				auto& light = lights[i];
				if (
					light->getCastShadow() &&
					light->getLightType() == LightType::Directional
				)
				{
					m_gatheredView.lights[0] = light;
					break;
				}
			}
		}

		// Add all other lights.
		for (int32_t i = 0; i < (int32_t)lights.size(); ++i)
		{
			auto& light = lights[i];
			if (light != m_gatheredView.lights[0])
				m_gatheredView.lights.push_back(light);
		}
	}
}

}
