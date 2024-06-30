/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"
#include "Weather/Sky/SkyComponentData.h"
#include "World/Entity/IIrradianceGridComponent.h"

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

namespace traktor::world
{

class IrradianceGrid;

}

namespace traktor::weather
{

/*! Sky background component.
 * \ingroup Weather
 */
class T_DLLCLASS SkyComponent : public world::IIrradianceGridComponent
{
	T_RTTI_CLASS;

public:
	explicit SkyComponent(
		const SkyComponentData& data,
		const world::IrradianceGrid* irradianceGrid,
		const resource::Proxy< render::Shader >& shader,
		const resource::Proxy< render::ITexture >& texture
	);

	virtual ~SkyComponent();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual const world::IrradianceGrid* getIrradianceGrid() const override final { return m_irradianceGrid; }

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
	const SkyComponentData m_data;
	Ref< const world::IrradianceGrid > m_irradianceGrid;
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
	int32_t m_count = 0;
	int32_t m_cloudFrame = 0;
	float m_cloudBlend = 0.0f;
	bool m_dirty = true;
};

}
