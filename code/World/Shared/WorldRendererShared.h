/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"

namespace traktor::render
{

class Buffer;
class ImageGraph;
class ImageGraphContext;
class IRenderTargetSet;
class ITexture;
class ScreenRenderer;

}

namespace traktor::world
{

class AmbientOcclusionPass;
class GBufferPass;
class IEntityRenderer;
class LightClusterPass;
class LightComponent;
class PostProcessPass;
class ProbeComponent;
class ReflectionsPass;
class VelocityPass;

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

    virtual render::ImageGraphContext* getImageGraphContext() const override final;

protected:
	WorldRenderSettings m_settings;

	Quality m_shadowsQuality = Quality::Disabled;

	int32_t m_count = 0;

    Ref< render::IRenderTargetSet > m_sharedDepthStencil;

	Ref< LightClusterPass > m_lightClusterPass;
	Ref< GBufferPass > m_gbufferPass;
	Ref< VelocityPass > m_velocityPass;
	Ref< AmbientOcclusionPass > m_ambientOcclusionPass;
	Ref< ReflectionsPass > m_reflectionsPass;
	Ref< PostProcessPass > m_postProcessPass;

    Ref< render::ImageGraphContext > m_imageGraphContext;

    Ref< WorldEntityRenderers > m_entityRenderers;
    Ref< render::ScreenRenderer > m_screenRenderer;

	Ref< render::ITexture > m_blackTexture;
	Ref< render::ITexture > m_whiteTexture;
	Ref< render::ITexture > m_blackCubeTexture;

	GatherView m_gatheredView;

	void gather(Entity* rootEntity);
};

}
