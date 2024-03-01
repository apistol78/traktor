/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IWorldRenderPass;
class WorldRenderView;
class WorldSetupContext;

}

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class IVertexLayout;
class RenderContext;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::weather
{

/*! Sky background component.
 * \ingroup Weather
 */
class T_DLLCLASS SkyComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit SkyComponent(
		const resource::Proxy< render::Shader >& shader,
		const resource::Proxy< render::ITexture >& texture,
		float intensity
	);

	virtual ~SkyComponent();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void setup(
		const world::WorldSetupContext& context,
		const world::WorldRenderView& worldRenderView
	);

	void build(
		render::RenderContext* renderContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

private:
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	render::Primitives m_primitives;
	resource::Proxy< render::Shader > m_shaderClouds2D;
	resource::Proxy< render::Shader > m_shaderClouds3D;
	resource::Proxy< render::Shader > m_shaderCloudsDome;
	resource::Proxy< render::Shader > m_shader;
	resource::Proxy< render::ITexture > m_texture;
	Ref< render::ITexture > m_cloudTextures[2];
	Ref< render::ITexture > m_cloudDomeTexture[2];
	world::Entity* m_owner = nullptr;
	Transform m_transform;
	float m_intensity = 1.0f;
	int32_t m_count = 0;
	int32_t m_cloudFrame = 0;
	bool m_dirty = true;
};

}
