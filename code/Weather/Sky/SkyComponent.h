/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

}

namespace traktor::render
{

class Buffer;
class RenderContext;
class ITexture;
class IVertexLayout;

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
		const render::IVertexLayout* vertexLayout,
		render::Buffer* vertexBuffer,
		render::Buffer* indexBuffer,
		const render::Primitives& primitives,
		const resource::Proxy< render::Shader >& shader,
		const resource::Proxy< render::ITexture >& texture
	);

	virtual ~SkyComponent();

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

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
	resource::Proxy< render::Shader > m_shader;
	resource::Proxy< render::ITexture > m_texture;
	Transform m_transform;
};

}
