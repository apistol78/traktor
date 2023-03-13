/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Weather/Fog/VolumetricFogComponent.h"
#include "Weather/Sky/SkyComponent.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

namespace traktor::weather
{
	namespace
	{

const render::Handle s_handleWeather_FogVolume(L"Weather_FogVolume");
const render::Handle s_handleWeather_SkyTexture(L"Weather_SkyTexture");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.VolumetricFogComponent", VolumetricFogComponent, IEntityComponent)

VolumetricFogComponent::VolumetricFogComponent(const resource::Proxy< render::Shader >& shader)
:	m_shader(shader)
{
}

bool VolumetricFogComponent::create(render::IRenderSystem* renderSystem)
{
	render::VolumeTextureCreateDesc vtcd;
	vtcd.width = 128;
	vtcd.height = 128;
	vtcd.depth = 128;
	vtcd.mipCount = 1;
	vtcd.format = render::TfR16G16B16A16F;
	vtcd.sRGB = false;
	vtcd.immutable = false;
	vtcd.shaderStorage = true;
	if ((m_fogVolumeTexture = renderSystem->createVolumeTexture(vtcd, T_FILE_LINE_W)) == nullptr)
		return false;

	return true;
}

void VolumetricFogComponent::destroy()
{
	safeDestroy(m_fogVolumeTexture);
}

void VolumetricFogComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void VolumetricFogComponent::setTransform(const Transform& transform)
{
}

Aabb3 VolumetricFogComponent::getBoundingBox() const
{
	return Aabb3();
}

void VolumetricFogComponent::update(const world::UpdateParams& update)
{
}

void VolumetricFogComponent::setup(const world::WorldSetupContext& context, const world::WorldRenderView& worldRenderView)
{
	if (!m_owner)
		return;

	SkyComponent* sky = m_owner->getComponent< SkyComponent >();
	if (!sky)
		return;

	const Frustum viewFrustum = worldRenderView.getViewFrustum();
	const Matrix44 projectionInv = worldRenderView.getProjection().inverse();

	auto& renderGraph = context.getRenderGraph();

	Ref< render::RenderPass > rp = new render::RenderPass(L"Volumetric fog");
	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			for (int32_t slice = 0; slice < 128; ++slice)
			{
				const float nf = (float)slice / 128.0f;
				const float ff = (float)(slice + 1) / 128.0f;

				const float sliceNearZ = (viewFrustum.getFarZ() - viewFrustum.getNearZ()) * nf + viewFrustum.getNearZ();
				const float sliceFarZ = (viewFrustum.getFarZ() - viewFrustum.getNearZ()) * ff + viewFrustum.getNearZ();

				auto renderBlock = renderContext->alloc< render::ComputeRenderBlock >(L"Volumetric fog, inject sky");

				renderBlock->program = m_shader->getProgram().program;
				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->workSize[0] = 128;
				renderBlock->workSize[1] = 128;
				renderBlock->workSize[2] = 1;

				renderBlock->programParams->beginParameters(renderContext);

				//worldRenderPass.setProgramParameters(renderBlock->programParams);

				renderBlock->programParams->setImageViewParameter(s_handleWeather_FogVolume, m_fogVolumeTexture);
				renderBlock->programParams->setTextureParameter(s_handleWeather_SkyTexture, sky->getTexture());

				renderBlock->programParams->setFloatParameter(render::getParameterHandle(L"Weather_Slice"), (float)slice);
				renderBlock->programParams->setFloatParameter(render::getParameterHandle(L"Weather_SliceNearZ"), sliceNearZ);
				renderBlock->programParams->setFloatParameter(render::getParameterHandle(L"Weather_SliceFarZ"), sliceFarZ);

				renderBlock->programParams->setMatrixParameter(render::getParameterHandle(L"Weather_ProjectionInv"), projectionInv);

				renderBlock->programParams->endParameters(renderContext);

				renderContext->compute(renderBlock);
			}
		}
	);
	renderGraph.addPass(rp);
}

void VolumetricFogComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{

}

}
