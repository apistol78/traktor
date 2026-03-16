/*
 * TRAKTOR
 * Copyright (c) 2023-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/AmbientOcclusionPass.h"

#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "World/IWorldRenderer.h"
#include "World/Shared/Passes/RT/RTAmbientOcclusionPass.h"
#include "World/Shared/Passes/SS/SSAmbientOcclusionPass.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.AmbientOcclusionPass", AmbientOcclusionPass, Object)

bool AmbientOcclusionPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (desc.quality.ambientOcclusion >= Quality::High && desc.rt && renderSystem->supportRayTracing())
	{
		m_rt = new RTAmbientOcclusionPass();
		if (!m_rt->create(resourceManager, renderSystem, desc))
		{
			m_rt = nullptr;
			return false;
		}
	}
	else if (desc.quality.ambientOcclusion > Quality::Disabled)
	{
		m_ss = new SSAmbientOcclusionPass();
		if (!m_ss->create(resourceManager, renderSystem, desc))
		{
			m_ss = nullptr;
			return false;
		}
	}
	return true;
}

void AmbientOcclusionPass::destroy()
{
	safeDestroy(m_rt);
	safeDestroy(m_ss);
}

render::RGTargetSet AmbientOcclusionPass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	bool needJitter,
	uint32_t frameCount,
	render::RenderGraph& renderGraph,
	render::RGTargetSet gbufferTargetSetId,
	render::RGTexture halfResDepthTextureId,
	render::RGTargetSet outputTargetSetId) const
{
	if (m_rt != nullptr)
		return m_rt->setup(worldRenderView, gatheredView, needJitter, frameCount, renderGraph, gbufferTargetSetId, halfResDepthTextureId, outputTargetSetId);
	else if (m_ss != nullptr)
		return m_ss->setup(worldRenderView, gatheredView, needJitter, frameCount, renderGraph, gbufferTargetSetId, halfResDepthTextureId, outputTargetSetId);
	else
		return render::RGTargetSet::Invalid;
}

}
