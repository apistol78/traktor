/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/FogComponent.h"

#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "World/Entity.h"
#include "World/Entity/FogComponentData.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

namespace traktor::world
{
namespace
{

const uint32_t c_interleave = 4;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.FogComponent", FogComponent, IEntityComponent)

FogComponent::FogComponent(const FogComponentData* data, const resource::Proxy< render::Shader >& shader)
	: m_shader(shader)
{
	m_fogDistance = data->m_fogDistance;
	m_fogDensity = data->m_fogDensity;
	m_fogDensityMax = data->m_fogDensityMax;
	m_fogColor = data->m_fogColor;

	m_volumetricFogEnable = data->m_volumetricFogEnable;
	m_maxDistance = data->m_maxDistance;
	m_maxScattering = data->m_maxScattering;
	m_sliceCount = alignUp(data->m_sliceCount, c_interleave);
	m_mediumColor = data->m_mediumColor;
	m_mediumDensity = data->m_mediumDensity;
}

bool FogComponent::create(render::IRenderSystem* renderSystem)
{
	if (m_volumetricFogEnable)
	{
		render::VolumeTextureCreateDesc vtcd;
		vtcd.width = 128;
		vtcd.height = 128;
		vtcd.depth = m_sliceCount;
		vtcd.mipCount = 1;
		vtcd.format = render::TfR16G16B16A16F;
		vtcd.sRGB = false;
		vtcd.immutable = false;
		vtcd.shaderStorage = true;
		if ((m_fogVolumeTexture = renderSystem->createVolumeTexture(vtcd, T_FILE_LINE_W)) == nullptr)
			return false;
	}
	return true;
}

void FogComponent::destroy()
{
	safeDestroy(m_fogVolumeTexture);
}

void FogComponent::setOwner(Entity* owner)
{
	m_owner = owner;
}

void FogComponent::setTransform(const Transform& transform)
{
}

Aabb3 FogComponent::getBoundingBox() const
{
	return Aabb3();
}

void FogComponent::update(const UpdateParams& update)
{
}

void FogComponent::build(const WorldBuildContext& context, const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass)
{
	if (!m_owner || !m_volumetricFogEnable || worldRenderView.getSnapshot())
		return;

	if (
		worldRenderPass.getTechnique() != ShaderTechnique::DeferredColor &&
		worldRenderPass.getTechnique() != ShaderTechnique::ForwardColor)
		return;

	auto permutation = worldRenderPass.getPermutation(m_shader);
	permutation.technique = ShaderTechnique::Default;
	auto injectLightsProgram = m_shader->getProgram(permutation);
	if (!injectLightsProgram)
		return;

	const Frustum viewFrustum = worldRenderView.getViewFrustum();
	const Vector4 fogRange(
		viewFrustum.getNearZ(),
		std::min< float >(viewFrustum.getFarZ(), m_maxDistance),
		0.0f,
		0.0f);

	const Scalar p11 = worldRenderView.getProjection().get(0, 0);
	const Scalar p22 = worldRenderView.getProjection().get(1, 1);

	auto renderContext = context.getRenderContext();
	auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Volumetric fog, inject analytical lights");

	renderBlock->program = injectLightsProgram.program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->workSize[0] = 128;
	renderBlock->workSize[1] = 128;
	renderBlock->workSize[2] = m_sliceCount / c_interleave;

	renderBlock->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(renderBlock->programParams);

	renderBlock->programParams->setImageViewParameter(ShaderParameter::FogVolume, m_fogVolumeTexture, 0);
	renderBlock->programParams->setVectorParameter(ShaderParameter::FogVolumeRange, fogRange);
	renderBlock->programParams->setVectorParameter(ShaderParameter::MagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
	renderBlock->programParams->setVectorParameter(ShaderParameter::FogVolumeMediumColor, m_mediumColor);
	renderBlock->programParams->setFloatParameter(ShaderParameter::FogVolumeMediumDensity, m_mediumDensity / m_sliceCount);
	renderBlock->programParams->setFloatParameter(ShaderParameter::FogVolumeSliceCount, (float)m_sliceCount);
	renderBlock->programParams->setFloatParameter(ShaderParameter::FogVolumeSliceCurrent, (float)m_sliceCurrent);
	renderBlock->programParams->endParameters(renderContext);

	renderContext->compute(renderBlock);
	renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Fragment, m_fogVolumeTexture, 0);

	m_sliceCurrent = (m_sliceCurrent + 1) % c_interleave;
}

}
