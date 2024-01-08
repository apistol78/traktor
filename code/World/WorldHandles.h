/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Types.h"

 // import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

// Techniques
extern const render::Handle T_DLLCLASS s_techniqueDeferredColor;
extern const render::Handle T_DLLCLASS s_techniqueDeferredGBufferWrite;
extern const render::Handle T_DLLCLASS s_techniqueForwardColor;
extern const render::Handle T_DLLCLASS s_techniqueForwardGBufferWrite;
extern const render::Handle T_DLLCLASS s_techniqueSimpleColor;
extern const render::Handle T_DLLCLASS s_techniqueDBufferWrite;
extern const render::Handle T_DLLCLASS s_techniqueReflectionWrite;
extern const render::Handle T_DLLCLASS s_techniqueVelocityWrite;
extern const render::Handle T_DLLCLASS s_techniqueShadow;

// Permutations
extern const render::Handle s_handleIrradianceEnable;
extern const render::Handle s_handleIrradianceSingle;
extern const render::Handle s_handleVolumetricFogEnable;

// Shader parameters.
extern const render::Handle s_handleIrradianceMap;
extern const render::Handle s_handleDecalParams;
extern const render::Handle s_handleExposure;
extern const render::Handle s_handleFxRotate;
extern const render::Handle s_handleExtent;
extern const render::Handle s_handleFogColor;
extern const render::Handle s_handleFogDistanceAndDensity;
extern const render::Handle s_handleGamma;
extern const render::Handle s_handleGammaInverse;
extern const render::Handle s_handleLastWorld;
extern const render::Handle s_handleLastWorldView;
extern const render::Handle s_handleLightDiffuseMap;
extern const render::Handle s_handleLightIndexSBuffer;
extern const render::Handle s_handleLightMap;
extern const render::Handle s_handleLightSBuffer;
extern const render::Handle s_handleLightSpecularMap;
extern const render::Handle s_handleMagicCoeffs;
extern const render::Handle s_handleOcclusionMap;
extern const render::Handle s_handleContactShadowsMap;
extern const render::Handle s_handleProjection;
extern const render::Handle s_handleReflectionMap;
extern const render::Handle s_handleScreenMap;
extern const render::Handle s_handleShadowMapAtlas;
extern const render::Handle s_handleShadowBias;
extern const render::Handle s_handleTileSBuffer;
extern const render::Handle s_handleTime;
extern const render::Handle s_handleViewDistance;
extern const render::Handle s_handleView;
extern const render::Handle s_handleViewInverse;
extern const render::Handle s_handleWorld;
extern const render::Handle s_handleWorldView;
extern const render::Handle s_handleWorldViewInv;
extern const render::Handle s_handleJitter;
extern const render::Handle s_handleSlicePositions;
extern const render::Handle s_handleGBufferColorMap;
extern const render::Handle s_handleGBufferDepthMap;
extern const render::Handle s_handleGBufferMiscMap;
extern const render::Handle s_handleGBufferNormalMap;
extern const render::Handle s_handleDBufferColorMap;
extern const render::Handle s_handleDBufferMiscMap;
extern const render::Handle s_handleDBufferNormalMap;

// Irradiance grid.
extern const render::Handle s_handleIrradianceGridBoundsMax;
extern const render::Handle s_handleIrradianceGridBoundsMin;
extern const render::Handle s_handleIrradianceGridSBuffer;
extern const render::Handle s_handleIrradianceGridSize;

// Reflection probe.
extern const render::Handle s_handleProbeDiffuse;
extern const render::Handle s_handleProbeIntensity;
extern const render::Handle s_handleProbeTexture;
extern const render::Handle s_handleProbeTextureMips;
extern const render::Handle s_handleProbeVolumeCenter;
extern const render::Handle s_handleProbeVolumeExtent;
extern const render::Handle s_handleProbeRoughness;
extern const render::Handle s_handleProbeFilterCorners;

// Volumetric fog.
extern const render::Handle s_handleFogVolume;
extern const render::Handle s_handleFogVolumeTexture;
extern const render::Handle s_handleFogVolumeRange;
extern const render::Handle s_handleFogVolumeSliceCount;
extern const render::Handle s_handleFogVolumeSliceCurrent;
extern const render::Handle s_handleFogVolumeMediumColor;
extern const render::Handle s_handleFogVolumeMediumDensity;

// Contact shadows.
extern const render::Handle s_handleContactLightDirection;

// ImageGraph inputs.
extern const render::Handle s_handleInputColor;
extern const render::Handle s_handleInputColorLast;
extern const render::Handle s_handleInputDepth;
extern const render::Handle s_handleInputNormal;
extern const render::Handle s_handleInputVelocity;
extern const render::Handle s_handleInputVelocityLast;
extern const render::Handle s_handleInputColorGrading;

// Persistent targets.
extern const render::Handle s_handleTargetShadowMap[];

}
