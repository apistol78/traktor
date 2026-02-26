/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/ReflectionsPass.h"

#include "Render/IRenderSystem.h"
#include "World/IWorldRenderer.h"
#include "World/Shared/Passes/RTReflectionsPass.h"
#include "World/Shared/Passes/SSReflectionsPass.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ReflectionsPass", ReflectionsPass, Object)

bool ReflectionsPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (desc.quality.reflections >= Quality::High && renderSystem->supportRayTracing())
	{
		m_rt = new RTReflectionsPass();
		if (!m_rt->create(resourceManager, renderSystem, desc))
		{
			m_rt = nullptr;
			return false;
		}
	}
	else if (desc.quality.reflections >= Quality::Disabled)
	{
		m_ss = new SSReflectionsPass();
		if (!m_ss->create(resourceManager, renderSystem, desc))
		{
			m_ss = nullptr;
			return false;
		}
	}
	return true;
}

render::RGTargetSet ReflectionsPass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	const render::Buffer* lightSBuffer,
	render::ITexture* blackCubeTexture,
	bool needJitter,
	uint32_t frameCount,
	render::RenderGraph& renderGraph,
	render::RGTargetSet gbufferTargetSetId,
	render::RGTargetSet dbufferTargetSetId,
	render::RGTargetSet visualReadTargetSetId,
	render::RGTargetSet velocityTargetSetId,
	render::RGTexture halfResDepthTextureId,
	render::RGTargetSet outputTargetSetId) const
{
	if (m_rt != nullptr)
		return m_rt->setup(worldRenderView, gatheredView, lightSBuffer, blackCubeTexture, needJitter, frameCount, renderGraph, gbufferTargetSetId, velocityTargetSetId, halfResDepthTextureId, outputTargetSetId);
	else if (m_ss != nullptr)
		return m_ss->setup(worldRenderView, gatheredView, lightSBuffer, needJitter, frameCount, renderGraph, gbufferTargetSetId, dbufferTargetSetId, visualReadTargetSetId, velocityTargetSetId, outputTargetSetId);
	else
		return render::RGTargetSet::Invalid;
}

}
