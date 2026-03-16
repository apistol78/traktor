/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/IrradiancePass.h"

#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "World/IWorldRenderer.h"
#include "World/Shared/Passes/RT/RTIrradiancePass.h"
#include "World/Shared/Passes/SS/SSIrradiancePass.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.IrradiancePass", IrradiancePass, Object)

bool IrradiancePass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (desc.quality.irradiance >= Quality::High && desc.rt && renderSystem->supportRayTracing())
	{
		m_rt = new RTIrradiancePass();
		if (!m_rt->create(resourceManager, renderSystem, desc))
		{
			m_rt = nullptr;
			return false;
		}
	}
	else if (desc.quality.irradiance > Quality::Disabled)
	{
		m_ss = new SSIrradiancePass();
		if (!m_ss->create(resourceManager, renderSystem, desc))
		{
			m_ss = nullptr;
			return false;
		}
	}
	return true;
}

void IrradiancePass::destroy()
{
	safeDestroy(m_rt);
	safeDestroy(m_ss);
}

render::RGTargetSet IrradiancePass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	const render::Buffer* lightSBuffer,
	bool needJitter,
	uint32_t frameCount,
	render::RenderGraph& renderGraph,
	render::RGTargetSet gbufferTargetSetId,
	render::RGTargetSet velocityTargetSetId,
	render::RGTexture halfResDepthTextureId,
	render::RGTargetSet outputTargetSetId) const
{
	if (m_rt != nullptr)
		return m_rt->setup(worldRenderView, gatheredView, lightSBuffer, needJitter, frameCount, renderGraph, gbufferTargetSetId, velocityTargetSetId, halfResDepthTextureId, outputTargetSetId);
	else if (m_ss != nullptr)
		return m_ss->setup(worldRenderView, gatheredView, lightSBuffer, needJitter, frameCount, renderGraph, gbufferTargetSetId, velocityTargetSetId, halfResDepthTextureId, outputTargetSetId);
	else
		return render::RGTargetSet::Invalid;
}

}
