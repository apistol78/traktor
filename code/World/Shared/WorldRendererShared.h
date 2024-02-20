/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Frustum.h"
#include "Resource/Proxy.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"

namespace traktor::render
{

class Buffer;
class IRenderTargetSet;
class ITexture;
class ScreenRenderer;
class Shader;

}

namespace traktor::world
{

class AmbientOcclusionPass;
class ContactShadowsPass;
class DBufferPass;
class GBufferPass;
class HiZPass;
class IEntityRenderer;
class IrradianceGrid;
class LightClusterPass;
class LightComponent;
class Packer;
class PostProcessPass;
class ProbeComponent;
class ReflectionsPass;
class VelocityPass;
class WorldEntityRenderers;

/*! World renderer shared implementation.
 * \ingroup World
 */
class T_DLLCLASS WorldRendererShared : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override;

	virtual void destroy() override;

protected:
	friend class SliceOverlay;
	friend class TilesOverlay;

#pragma pack(1)
	struct LightShaderData
	{
		float type[4];
		float rangeRadius[4];
		float position[4];
		float direction[4];
		float color[4];
		float viewToLight0[4];
		float viewToLight1[4];
		float viewToLight2[4];
		float viewToLight3[4];
		float atlasTransform[4];
	};
#pragma pack()

	WorldRenderSettings m_settings;

	Quality m_shadowsQuality = Quality::Disabled;
	float m_slicePositions[MaxSliceCount + 1];

	Ref< LightClusterPass > m_lightClusterPass;
	Ref< GBufferPass > m_gbufferPass;
	Ref< DBufferPass > m_dbufferPass;
	Ref< HiZPass > m_hiZPass;
	Ref< VelocityPass > m_velocityPass;
	Ref< AmbientOcclusionPass > m_ambientOcclusionPass;
	Ref< ContactShadowsPass > m_contactShadowsPass;
	Ref< ReflectionsPass > m_reflectionsPass;
	Ref< PostProcessPass > m_postProcessPass;

    Ref< WorldEntityRenderers > m_entityRenderers;
    Ref< render::ScreenRenderer > m_screenRenderer;

	Ref< render::ITexture > m_blackTexture;
	Ref< render::ITexture > m_whiteTexture;
	Ref< render::ITexture > m_blackCubeTexture;

	resource::Proxy< render::Shader > m_clearDepthShader;

	resource::Proxy< IrradianceGrid > m_irradianceGrid;
	Ref< Packer > m_shadowAtlasPacker;

	GatherView m_gatheredView;
	AlignedVector< render::handle_t > m_visualAttachments;

	struct State
	{
		Ref< render::Buffer > lightSBuffer;
		Frustum shadowSlices[4];
		Matrix44 shadowLightViews[4];
		uint32_t count = 0;
	};
	State m_state[4];

	void gather(const World* world, const std::function< bool(const EntityState& state) >& filter);

	void setupLightPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t& outShadowMapAtlasTargetSetId
	);
};

}
