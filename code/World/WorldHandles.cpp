/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/WorldHandles.h"

namespace traktor::world
{

// Techniques
const render::Handle s_techniqueDeferredColor(L"World_DeferredColor");
const render::Handle s_techniqueDeferredGBufferWrite(L"World_DeferredGBufferWrite");
const render::Handle s_techniqueForwardColor(L"World_ForwardColor");
const render::Handle s_techniqueForwardGBufferWrite(L"World_ForwardGBufferWrite");
const render::Handle s_techniqueSimpleColor(L"World_SimpleColor");
const render::Handle s_techniqueDBufferWrite(L"World_DBufferWrite");
const render::Handle s_techniqueReflectionWrite(L"World_ReflectionWrite");
const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");
const render::Handle s_techniqueShadow(L"World_ShadowWrite");

// Permutations
const render::Handle s_handleIrradianceEnable(L"World_IrradianceEnable");
const render::Handle s_handleIrradianceSingle(L"World_IrradianceSingle");
const render::Handle s_handleVolumetricFogEnable(L"World_VolumetricFogEnable");

// Shader parameters.
const render::Handle s_handleDecalParamsA(L"World_DecalParamsA");
const render::Handle s_handleDecalParamsB(L"World_DecalParamsB");
const render::Handle s_handleExposure(L"World_Exposure");
const render::Handle s_handleFxRotate(L"World_FxRotate");
const render::Handle s_handleExtent(L"World_Extent");
const render::Handle s_handleFogColor(L"World_FogColor");
const render::Handle s_handleFogDistanceAndDensity(L"World_FogDistanceAndDensity");
const render::Handle s_handleGamma(L"World_Gamma");
const render::Handle s_handleGammaInverse(L"World_GammaInverse");
const render::Handle s_handleLastWorld(L"World_LastWorld");
const render::Handle s_handleLastView(L"World_LastView");
const render::Handle s_handleLastWorldView(L"World_LastWorldView");
const render::Handle s_handleLightDiffuseMap(L"World_LightDiffuseMap");
const render::Handle s_handleLightIndexSBuffer(L"World_LightIndexSBuffer");
const render::Handle s_handleLightMap(L"World_LightMap");
const render::Handle s_handleLightSBuffer(L"World_LightSBuffer");
const render::Handle s_handleLightSpecularMap(L"World_LightSpecularMap");
const render::Handle s_handleMagicCoeffs(L"World_MagicCoeffs");
const render::Handle s_handleOcclusionMap(L"World_OcclusionMap");
const render::Handle s_handleContactShadowsMap(L"World_ContactShadowsMap");
const render::Handle s_handleProjection(L"World_Projection");
const render::Handle s_handleReflectionMap(L"World_ReflectionMap");
const render::Handle s_handleVisualCopyMap(L"World_VisualCopyMap");
const render::Handle s_handleShadowMapAtlas(L"World_ShadowMapAtlas");
const render::Handle s_handleShadowBias(L"World_ShadowBias");
const render::Handle s_handleTileSBuffer(L"World_TileSBuffer");
const render::Handle s_handleTime(L"World_Time");
const render::Handle s_handleViewDistance(L"World_ViewDistance");
const render::Handle s_handleView(L"World_View");
const render::Handle s_handleViewInverse(L"World_ViewInverse");
const render::Handle s_handleWorld(L"World_World");
const render::Handle s_handleWorldView(L"World_WorldView");
const render::Handle s_handleWorldViewInv(L"World_WorldViewInv");
const render::Handle s_handleJitter(L"World_Jitter");
const render::Handle s_handleSlicePositions(L"World_SlicePositions");
const render::Handle s_handleGBufferA(L"World_GBufferA");
const render::Handle s_handleGBufferB(L"World_GBufferB");
const render::Handle s_handleGBufferC(L"World_GBufferC");
const render::Handle s_handleGBufferD(L"World_GBufferD");
const render::Handle s_handleDBufferColorMap(L"World_DBufferColorMap");
const render::Handle s_handleDBufferMiscMap(L"World_DBufferMiscMap");
const render::Handle s_handleDBufferNormalMap(L"World_DBufferNormalMap");
const render::Handle s_handleHiZTexture(L"World_HiZTexture");

// Irradiance grid.
const render::Handle s_handleIrradianceGridBoundsMax(L"World_IrradianceGridBoundsMax");
const render::Handle s_handleIrradianceGridBoundsMin(L"World_IrradianceGridBoundsMin");
const render::Handle s_handleIrradianceGridSBuffer(L"World_IrradianceGridSBuffer");
const render::Handle s_handleIrradianceGridSize(L"World_IrradianceGridSize");

// Reflection probe.
const render::Handle s_handleProbeDiffuse(L"World_ProbeDiffuse");
const render::Handle s_handleProbeIntensity(L"World_ProbeIntensity");
const render::Handle s_handleProbeTexture(L"World_ProbeTexture");
const render::Handle s_handleProbeTextureMips(L"World_ProbeTextureMips");
const render::Handle s_handleProbeVolumeCenter(L"World_ProbeVolumeCenter");
const render::Handle s_handleProbeVolumeExtent(L"World_ProbeVolumeExtent");
const render::Handle s_handleProbeRoughness(L"World_ProbeRoughness");
const render::Handle s_handleProbeFilterCorners(L"World_ProbeFilterCorners");

// Volumetric fog.
const render::Handle s_handleFogVolume(L"World_FogVolume");
const render::Handle s_handleFogVolumeTexture(L"World_FogVolumeTexture");
const render::Handle s_handleFogVolumeRange(L"World_FogVolumeRange");
const render::Handle s_handleFogVolumeSliceCount(L"World_FogVolumeSliceCount");
const render::Handle s_handleFogVolumeSliceCurrent(L"World_FogVolumeSliceCurrent");
const render::Handle s_handleFogVolumeMediumColor(L"World_FogVolumeMediumColor");
const render::Handle s_handleFogVolumeMediumDensity(L"World_FogVolumeMediumDensity");

// Contact shadows.
const render::Handle s_handleContactLightDirection(L"World_ContactLightDirection");

// Occlusion culling.
const render::Handle s_handleCullingHiZ(L"World_CullingHiZ");

// ImageGraph inputs.
const render::Handle s_handleInputColor(L"InputColor");
const render::Handle s_handleInputColorLast(L"InputColorLast");
const render::Handle s_handleInputDepth(L"InputDepth");
const render::Handle s_handleInputNormal(L"InputNormal");
const render::Handle s_handleInputVelocity(L"InputVelocity");
const render::Handle s_handleInputColorGrading(L"InputColorGrading");

// Persistent targets.
const render::Handle s_handleTargetShadowMap[] =
{
    render::Handle(L"World_ShadowMap0"),
    render::Handle(L"World_ShadowMap1"),
    render::Handle(L"World_ShadowMap2"),
    render::Handle(L"World_ShadowMap3")
};

}
