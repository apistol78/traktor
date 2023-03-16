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
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Weather/Fog/VolumetricFogComponent.h"
#include "Weather/Sky/SkyComponent.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

namespace traktor::weather
{
	namespace
	{

const render::Handle s_handleWeather_FogVolume(L"Weather_FogVolume");
const render::Handle s_handleWeather_FogVolumeTexture(L"Weather_FogVolumeTexture");
const render::Handle s_handleWeather_SkyTexture(L"Weather_SkyTexture");
const render::Handle s_handleWeather_MagicCoeffs(L"Weather_MagicCoeffs");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.VolumetricFogComponent", VolumetricFogComponent, IEntityComponent)

VolumetricFogComponent::VolumetricFogComponent(
	const resource::Proxy< render::Shader >& shader
)
:	m_shader(shader)
{
}

bool VolumetricFogComponent::create(render::IRenderSystem* renderSystem)
{
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

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
	safeDestroy(m_screenRenderer);
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
}

void VolumetricFogComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (worldRenderView.getSnapshot())
		return;

	auto renderContext = context.getRenderContext();

	if (!m_owner)
		return;

	SkyComponent* sky = m_owner->getComponent< SkyComponent >();
	if (!sky)
		return;

	auto permutation = worldRenderPass.getPermutation(m_shader);
	permutation.technique = render::getParameterHandle(L"InjectSky");
	auto injectSkyProgram = m_shader->getProgram(permutation);
	if (!injectSkyProgram)
		return;

	const Frustum viewFrustum = worldRenderView.getViewFrustum();
	const Matrix44 projectionInv = worldRenderView.getProjection().inverse();

	const Scalar p11 = worldRenderView.getProjection().get(0, 0);
	const Scalar p22 = worldRenderView.getProjection().get(1, 1);

	// Update the volume.
	if (worldRenderPass.getTechnique() == world::s_techniqueForwardColor)
	{
		auto renderBlock = renderContext->alloc< render::ComputeRenderBlock >(L"Volumetric fog, inject sky");

		renderBlock->program = injectSkyProgram.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->workSize[0] = 128;
		renderBlock->workSize[1] = 128;
		renderBlock->workSize[2] = 128;

		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(renderBlock->programParams);

		renderBlock->programParams->setImageViewParameter(s_handleWeather_FogVolume, m_fogVolumeTexture);
		renderBlock->programParams->setTextureParameter(s_handleWeather_SkyTexture, sky->getTexture());
		renderBlock->programParams->setVectorParameter(s_handleWeather_MagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, viewFrustum.getNearZ(), viewFrustum.getFarZ()));
		renderBlock->programParams->setMatrixParameter(render::getParameterHandle(L"Weather_ProjectionInverse"), projectionInv);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->compute(renderBlock);
	}

	// Render the volume.
	{
		auto perm = worldRenderPass.getPermutation(m_shader);

		auto programParams = renderContext->alloc< render::ProgramParameters >();
		programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(programParams);

		programParams->setTextureParameter(s_handleWeather_FogVolumeTexture, m_fogVolumeTexture);
		programParams->endParameters(renderContext);

		m_screenRenderer->draw(renderContext, m_shader, perm, programParams);
	}
}

}
