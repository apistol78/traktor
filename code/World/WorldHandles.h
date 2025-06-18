/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
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
struct T_DLLCLASS ShaderTechnique
{
	static const render::Handle Default;
	static const render::Handle DeferredColor;
	static const render::Handle DeferredGBufferWrite;
	static const render::Handle ForwardColor;
	static const render::Handle ForwardGBufferWrite;
	static const render::Handle SimpleColor;
	static const render::Handle DBufferWrite;
	static const render::Handle ReflectionWrite;
	static const render::Handle VelocityWrite;
	static const render::Handle Shadow;
};

// Permutations
struct T_DLLCLASS ShaderPermutation
{
	static const render::Handle IrradianceEnable;
	static const render::Handle IrradianceSingle;
	static const render::Handle VolumetricFogEnable;
	static const render::Handle ColorGradingEnable;
	static const render::Handle RayTracingEnable;
	static const render::Handle CullingHiZ;
	static const render::Handle HDR;
};

// Shader parameters.
struct T_DLLCLASS ShaderParameter
{
	static const render::Handle DecalParamsA;
	static const render::Handle DecalParamsB;
	static const render::Handle Exposure;
	static const render::Handle Extent;
	static const render::Handle FogColor;
	static const render::Handle FogDistanceAndDensity;
	static const render::Handle Gamma;
	static const render::Handle GammaInverse;
	static const render::Handle LastWorld;
	static const render::Handle LastView;
	static const render::Handle LastWorldView;
	static const render::Handle LightIndexSBuffer;
	static const render::Handle LightMap;
	static const render::Handle LightSBuffer;
	static const render::Handle LightCount;
	static const render::Handle MagicCoeffs;
	static const render::Handle IrradianceMap;
	static const render::Handle OcclusionMap;
	static const render::Handle ContactShadowsMap;
	static const render::Handle Projection;
	static const render::Handle ReflectionMap;
	static const render::Handle VisualCopyMap;
	static const render::Handle ShadowMapAtlas;
	static const render::Handle ShadowBias;
	static const render::Handle TileSBuffer;
	static const render::Handle Time;
	static const render::Handle Random;
	static const render::Handle ViewDistance;
	static const render::Handle View;
	static const render::Handle ViewInverse;
	static const render::Handle World;
	static const render::Handle WorldView;
	static const render::Handle WorldViewInv;
	static const render::Handle Jitter;
	static const render::Handle SlicePositions;
	static const render::Handle GBufferA;
	static const render::Handle GBufferB;
	static const render::Handle GBufferC;
	static const render::Handle DBufferColorMap;
	static const render::Handle DBufferMiscMap;
	static const render::Handle DBufferNormalMap;
	static const render::Handle HiZTexture;
	static const render::Handle TLAS;
	static const render::Handle HalfResDepthMap;
	static const render::Handle Reservoir;
	static const render::Handle ReservoirOutput;
	static const render::Handle VelocityMap;

	// Culling.
	static const render::Handle TargetSize;
	static const render::Handle Visibility;
	static const render::Handle CullFrustum;

	// Irradiance grid.
	static const render::Handle IrradianceGridBoundsMax;
	static const render::Handle IrradianceGridBoundsMin;
	static const render::Handle IrradianceGridSBuffer;
	static const render::Handle IrradianceGridSize;

	// Reflection probe.
	static const render::Handle ProbeDiffuse;
	static const render::Handle ProbeIntensity;
	static const render::Handle ProbeTexture;
	static const render::Handle ProbeTextureMips;
	static const render::Handle ProbeVolumeCenter;
	static const render::Handle ProbeVolumeExtent;
	static const render::Handle ProbeRoughness;
	static const render::Handle ProbeFilterCorners;

	// Volumetric fog.
	static const render::Handle FogVolume;
	static const render::Handle FogVolumeTexture;
	static const render::Handle FogVolumeRange;
	static const render::Handle FogVolumeSliceCount;
	static const render::Handle FogVolumeSliceCurrent;
	static const render::Handle FogVolumeMediumColor;
	static const render::Handle FogVolumeMediumDensity;

	// Contact shadows.
	static const render::Handle ContactLightDirection;

	// ImageGraph inputs.
	static const render::Handle InputColor;
	static const render::Handle InputColorLast;
	static const render::Handle InputDepth;
	static const render::Handle InputNormal;
	static const render::Handle InputVelocity;
	static const render::Handle InputColorGrading;

	// Persistent targets.
	static const render::Handle TargetShadowMap[];
};

}
