#pragma once

#include "Render/Types.h"

namespace traktor::world
{

// Techniques
extern const render::Handle s_techniqueDeferredColor;
extern const render::Handle s_techniqueDeferredGBufferWrite;
extern const render::Handle s_techniqueForwardColor;
extern const render::Handle s_techniqueForwardGBufferWrite;
extern const render::Handle s_techniqueSimpleColor;
extern const render::Handle s_techniqueReflectionWrite;
extern const render::Handle s_techniqueIrradianceWrite;
extern const render::Handle s_techniqueVelocityWrite;
extern const render::Handle s_techniqueShadow;

// Shader parameters.
extern const render::Handle s_handleColorMap;
extern const render::Handle s_handleDecalParams;
extern const render::Handle s_handleDepthMap;
extern const render::Handle s_handleExposure;
extern const render::Handle s_handleExtent;
extern const render::Handle s_handleFogColor;
extern const render::Handle s_handleFogDistanceAndDensity;
extern const render::Handle s_handleGamma;
extern const render::Handle s_handleGammaInverse;
extern const render::Handle s_handleIrradianceEnable;
extern const render::Handle s_handleIrradianceGridBoundsMax;
extern const render::Handle s_handleIrradianceGridBoundsMin;
extern const render::Handle s_handleIrradianceGridSBuffer;
extern const render::Handle s_handleIrradianceGridSize;
extern const render::Handle s_handleLastWorld;
extern const render::Handle s_handleLastWorldView;
extern const render::Handle s_handleLightCount;
extern const render::Handle s_handleLightDiffuseMap;
extern const render::Handle s_handleLightIndexSBuffer;
extern const render::Handle s_handleLightMap;
extern const render::Handle s_handleLightSBuffer;
extern const render::Handle s_handleLightSpecularMap;
extern const render::Handle s_handleMagicCoeffs;
extern const render::Handle s_handleMiscMap;
extern const render::Handle s_handleNormalMap;
extern const render::Handle s_handleOcclusionMap;
extern const render::Handle s_handleProbeDiffuse;
extern const render::Handle s_handleProbeIntensity;
extern const render::Handle s_handleProbeLocal;
extern const render::Handle s_handleProbeSpecular;
extern const render::Handle s_handleProbeSpecularMips;
extern const render::Handle s_handleProbeTexture;
extern const render::Handle s_handleProbeTextureMips;
extern const render::Handle s_handleProbeVolumeCenter;
extern const render::Handle s_handleProbeVolumeExtent;
extern const render::Handle s_handleProjection;
extern const render::Handle s_handleReflectionMap;
extern const render::Handle s_handleReflectionsEnable;
extern const render::Handle s_handleScreenMap;
extern const render::Handle s_handleShadowEnable;
extern const render::Handle s_handleShadowMapAtlas;
extern const render::Handle s_handleShadowMapCascade;
extern const render::Handle s_handleShadowMask;
extern const render::Handle s_handleTileSBuffer;
extern const render::Handle s_handleTime;
extern const render::Handle s_handleViewDistance;
extern const render::Handle s_handleView;
extern const render::Handle s_handleViewInverse;
extern const render::Handle s_handleWorld;
extern const render::Handle s_handleWorldView;
extern const render::Handle s_handleWorldViewInv;
extern const render::Handle s_handleJitter;

// ImageGraph inputs.
extern const render::Handle s_handleInputColor;
extern const render::Handle s_handleInputColorLast;
extern const render::Handle s_handleInputDepth;
extern const render::Handle s_handleInputNormal;
extern const render::Handle s_handleInputVelocity;
extern const render::Handle s_handleInputShadowMap;
extern const render::Handle s_handleInputRoughness;

}
