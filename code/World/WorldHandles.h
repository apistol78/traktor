/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
extern const render::Handle T_DLLCLASS s_handleIrradianceEnable;
extern const render::Handle T_DLLCLASS s_handleIrradianceSingle;
extern const render::Handle T_DLLCLASS s_handleVolumetricFogEnable;

// Shader parameters.
extern const render::Handle T_DLLCLASS s_handleDecalParams;
extern const render::Handle T_DLLCLASS s_handleExposure;
extern const render::Handle T_DLLCLASS s_handleFxRotate;
extern const render::Handle T_DLLCLASS s_handleExtent;
extern const render::Handle T_DLLCLASS s_handleFogColor;
extern const render::Handle T_DLLCLASS s_handleFogDistanceAndDensity;
extern const render::Handle T_DLLCLASS s_handleGamma;
extern const render::Handle T_DLLCLASS s_handleGammaInverse;
extern const render::Handle T_DLLCLASS s_handleLastWorld;
extern const render::Handle T_DLLCLASS s_handleLastWorldView;
extern const render::Handle T_DLLCLASS s_handleLightDiffuseMap;
extern const render::Handle T_DLLCLASS s_handleLightIndexSBuffer;
extern const render::Handle T_DLLCLASS s_handleLightMap;
extern const render::Handle T_DLLCLASS s_handleLightSBuffer;
extern const render::Handle T_DLLCLASS s_handleLightSpecularMap;
extern const render::Handle T_DLLCLASS s_handleMagicCoeffs;
extern const render::Handle T_DLLCLASS s_handleOcclusionMap;
extern const render::Handle T_DLLCLASS s_handleContactShadowsMap;
extern const render::Handle T_DLLCLASS s_handleProjection;
extern const render::Handle T_DLLCLASS s_handleReflectionMap;
extern const render::Handle T_DLLCLASS s_handleScreenMap;
extern const render::Handle T_DLLCLASS s_handleShadowMapAtlas;
extern const render::Handle T_DLLCLASS s_handleShadowBias;
extern const render::Handle T_DLLCLASS s_handleTileSBuffer;
extern const render::Handle T_DLLCLASS s_handleTime;
extern const render::Handle T_DLLCLASS s_handleViewDistance;
extern const render::Handle T_DLLCLASS s_handleView;
extern const render::Handle T_DLLCLASS s_handleViewInverse;
extern const render::Handle T_DLLCLASS s_handleWorld;
extern const render::Handle T_DLLCLASS s_handleWorldView;
extern const render::Handle T_DLLCLASS s_handleWorldViewInv;
extern const render::Handle T_DLLCLASS s_handleJitter;
extern const render::Handle T_DLLCLASS s_handleSlicePositions;
extern const render::Handle T_DLLCLASS s_handleGBufferA;
extern const render::Handle T_DLLCLASS s_handleGBufferB;
extern const render::Handle T_DLLCLASS s_handleGBufferC;
extern const render::Handle T_DLLCLASS s_handleGBufferD;
extern const render::Handle T_DLLCLASS s_handleDBufferColorMap;
extern const render::Handle T_DLLCLASS s_handleDBufferMiscMap;
extern const render::Handle T_DLLCLASS s_handleDBufferNormalMap;
extern const render::Handle T_DLLCLASS s_handleHiZTexture;

// Irradiance grid.
extern const render::Handle T_DLLCLASS s_handleIrradianceGridBoundsMax;
extern const render::Handle T_DLLCLASS s_handleIrradianceGridBoundsMin;
extern const render::Handle T_DLLCLASS s_handleIrradianceGridSBuffer;
extern const render::Handle T_DLLCLASS s_handleIrradianceGridSize;

// Reflection probe.
extern const render::Handle T_DLLCLASS s_handleProbeDiffuse;
extern const render::Handle T_DLLCLASS s_handleProbeIntensity;
extern const render::Handle T_DLLCLASS s_handleProbeTexture;
extern const render::Handle T_DLLCLASS s_handleProbeTextureMips;
extern const render::Handle T_DLLCLASS s_handleProbeVolumeCenter;
extern const render::Handle T_DLLCLASS s_handleProbeVolumeExtent;
extern const render::Handle T_DLLCLASS s_handleProbeRoughness;
extern const render::Handle T_DLLCLASS s_handleProbeFilterCorners;

// Volumetric fog.
extern const render::Handle T_DLLCLASS s_handleFogVolume;
extern const render::Handle T_DLLCLASS s_handleFogVolumeTexture;
extern const render::Handle T_DLLCLASS s_handleFogVolumeRange;
extern const render::Handle T_DLLCLASS s_handleFogVolumeSliceCount;
extern const render::Handle T_DLLCLASS s_handleFogVolumeSliceCurrent;
extern const render::Handle T_DLLCLASS s_handleFogVolumeMediumColor;
extern const render::Handle T_DLLCLASS s_handleFogVolumeMediumDensity;

// Contact shadows.
extern const render::Handle T_DLLCLASS s_handleContactLightDirection;

// ImageGraph inputs.
extern const render::Handle T_DLLCLASS s_handleInputColor;
extern const render::Handle T_DLLCLASS s_handleInputColorLast;
extern const render::Handle T_DLLCLASS s_handleInputDepth;
extern const render::Handle T_DLLCLASS s_handleInputNormal;
extern const render::Handle T_DLLCLASS s_handleInputVelocity;
extern const render::Handle T_DLLCLASS s_handleInputColorGrading;

// Persistent targets.
extern const render::Handle T_DLLCLASS s_handleTargetShadowMap[];

}
