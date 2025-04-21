/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/WorldHandles.h"

namespace traktor::world
{

// Techniques
const render::Handle ShaderTechnique::Default(L"Default");
const render::Handle ShaderTechnique::DeferredColor(L"World_DeferredColor");
const render::Handle ShaderTechnique::DeferredGBufferWrite(L"World_DeferredGBufferWrite");
const render::Handle ShaderTechnique::ForwardColor(L"World_ForwardColor");
const render::Handle ShaderTechnique::ForwardGBufferWrite(L"World_ForwardGBufferWrite");
const render::Handle ShaderTechnique::SimpleColor(L"World_SimpleColor");
const render::Handle ShaderTechnique::DBufferWrite(L"World_DBufferWrite");
const render::Handle ShaderTechnique::ReflectionWrite(L"World_ReflectionWrite");
const render::Handle ShaderTechnique::VelocityWrite(L"World_VelocityWrite");
const render::Handle ShaderTechnique::Shadow(L"World_ShadowWrite");

// Permutations
const render::Handle ShaderPermutation::IrradianceEnable(L"World_IrradianceEnable");
const render::Handle ShaderPermutation::IrradianceSingle(L"World_IrradianceSingle");
const render::Handle ShaderPermutation::VolumetricFogEnable(L"World_VolumetricFogEnable");
const render::Handle ShaderPermutation::ColorGradingEnable(L"World_ColorGradingEnable");
const render::Handle ShaderPermutation::RayTracingEnable(L"World_RayTracingEnable");
const render::Handle ShaderPermutation::CullingHiZ(L"World_CullingHiZ");

// Shader parameters.
const render::Handle ShaderParameter::DecalParamsA(L"World_DecalParamsA");
const render::Handle ShaderParameter::DecalParamsB(L"World_DecalParamsB");
const render::Handle ShaderParameter::Exposure(L"World_Exposure");
const render::Handle ShaderParameter::Extent(L"World_Extent");
const render::Handle ShaderParameter::FogColor(L"World_FogColor");
const render::Handle ShaderParameter::FogDistanceAndDensity(L"World_FogDistanceAndDensity");
const render::Handle ShaderParameter::Gamma(L"World_Gamma");
const render::Handle ShaderParameter::GammaInverse(L"World_GammaInverse");
const render::Handle ShaderParameter::LastWorld(L"World_LastWorld");
const render::Handle ShaderParameter::LastView(L"World_LastView");
const render::Handle ShaderParameter::LastWorldView(L"World_LastWorldView");
const render::Handle ShaderParameter::LightIndexSBuffer(L"World_LightIndexSBuffer");
const render::Handle ShaderParameter::LightMap(L"World_LightMap");
const render::Handle ShaderParameter::LightSBuffer(L"World_LightSBuffer");
const render::Handle ShaderParameter::LightCount(L"World_LightCount");
const render::Handle ShaderParameter::MagicCoeffs(L"World_MagicCoeffs");
const render::Handle ShaderParameter::IrradianceMap(L"World_IrradianceMap");
const render::Handle ShaderParameter::OcclusionMap(L"World_OcclusionMap");
const render::Handle ShaderParameter::ContactShadowsMap(L"World_ContactShadowsMap");
const render::Handle ShaderParameter::Projection(L"World_Projection");
const render::Handle ShaderParameter::ReflectionMap(L"World_ReflectionMap");
const render::Handle ShaderParameter::VisualCopyMap(L"World_VisualCopyMap");
const render::Handle ShaderParameter::ShadowMapAtlas(L"World_ShadowMapAtlas");
const render::Handle ShaderParameter::ShadowBias(L"World_ShadowBias");
const render::Handle ShaderParameter::TileSBuffer(L"World_TileSBuffer");
const render::Handle ShaderParameter::Time(L"World_Time");
const render::Handle ShaderParameter::Random(L"World_Random");
const render::Handle ShaderParameter::ViewDistance(L"World_ViewDistance");
const render::Handle ShaderParameter::View(L"World_View");
const render::Handle ShaderParameter::ViewInverse(L"World_ViewInverse");
const render::Handle ShaderParameter::World(L"World_World");
const render::Handle ShaderParameter::WorldView(L"World_WorldView");
const render::Handle ShaderParameter::WorldViewInv(L"World_WorldViewInv");
const render::Handle ShaderParameter::Jitter(L"World_Jitter");
const render::Handle ShaderParameter::SlicePositions(L"World_SlicePositions");
const render::Handle ShaderParameter::GBufferA(L"World_GBufferA");
const render::Handle ShaderParameter::GBufferB(L"World_GBufferB");
const render::Handle ShaderParameter::GBufferC(L"World_GBufferC");
const render::Handle ShaderParameter::DBufferColorMap(L"World_DBufferColorMap");
const render::Handle ShaderParameter::DBufferMiscMap(L"World_DBufferMiscMap");
const render::Handle ShaderParameter::DBufferNormalMap(L"World_DBufferNormalMap");
const render::Handle ShaderParameter::HiZTexture(L"World_HiZTexture");
const render::Handle ShaderParameter::TLAS(L"World_TLAS");
const render::Handle ShaderParameter::HalfResDepthMap(L"World_HalfResDepthMap");
const render::Handle ShaderParameter::Reservoir(L"World_Reservoir");
const render::Handle ShaderParameter::ReservoirOutput(L"World_ReservoirOutput");
const render::Handle ShaderParameter::VelocityMap(L"World_VelocityMap");

// Culling.
const render::Handle ShaderParameter::TargetSize(L"World_TargetSize");
const render::Handle ShaderParameter::Visibility(L"World_Visibility");
const render::Handle ShaderParameter::CullFrustum(L"World_CullFrustum");

// Irradiance grid.
const render::Handle ShaderParameter::IrradianceGridBoundsMax(L"World_IrradianceGridBoundsMax");
const render::Handle ShaderParameter::IrradianceGridBoundsMin(L"World_IrradianceGridBoundsMin");
const render::Handle ShaderParameter::IrradianceGridSBuffer(L"World_IrradianceGridSBuffer");
const render::Handle ShaderParameter::IrradianceGridSize(L"World_IrradianceGridSize");

// Reflection probe.
const render::Handle ShaderParameter::ProbeDiffuse(L"World_ProbeDiffuse");
const render::Handle ShaderParameter::ProbeIntensity(L"World_ProbeIntensity");
const render::Handle ShaderParameter::ProbeTexture(L"World_ProbeTexture");
const render::Handle ShaderParameter::ProbeTextureMips(L"World_ProbeTextureMips");
const render::Handle ShaderParameter::ProbeVolumeCenter(L"World_ProbeVolumeCenter");
const render::Handle ShaderParameter::ProbeVolumeExtent(L"World_ProbeVolumeExtent");
const render::Handle ShaderParameter::ProbeRoughness(L"World_ProbeRoughness");
const render::Handle ShaderParameter::ProbeFilterCorners(L"World_ProbeFilterCorners");

// Volumetric fog.
const render::Handle ShaderParameter::FogVolume(L"World_FogVolume");
const render::Handle ShaderParameter::FogVolumeTexture(L"World_FogVolumeTexture");
const render::Handle ShaderParameter::FogVolumeRange(L"World_FogVolumeRange");
const render::Handle ShaderParameter::FogVolumeSliceCount(L"World_FogVolumeSliceCount");
const render::Handle ShaderParameter::FogVolumeSliceCurrent(L"World_FogVolumeSliceCurrent");
const render::Handle ShaderParameter::FogVolumeMediumColor(L"World_FogVolumeMediumColor");
const render::Handle ShaderParameter::FogVolumeMediumDensity(L"World_FogVolumeMediumDensity");

// Contact shadows.
const render::Handle ShaderParameter::ContactLightDirection(L"World_ContactLightDirection");

// ImageGraph inputs.
const render::Handle ShaderParameter::InputColor(L"InputColor");
const render::Handle ShaderParameter::InputColorLast(L"InputColorLast");
const render::Handle ShaderParameter::InputDepth(L"InputDepth");
const render::Handle ShaderParameter::InputNormal(L"InputNormal");
const render::Handle ShaderParameter::InputVelocity(L"InputVelocity");
const render::Handle ShaderParameter::InputColorGrading(L"InputColorGrading");

// Persistent targets.
const render::Handle ShaderParameter::TargetShadowMap[] = {
	render::Handle(L"World_ShadowMap0"),
	render::Handle(L"World_ShadowMap1"),
	render::Handle(L"World_ShadowMap2"),
	render::Handle(L"World_ShadowMap3")
};

}
