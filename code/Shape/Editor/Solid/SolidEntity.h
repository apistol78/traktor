/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Buffer;
class IRenderSystem;
class IVertexLayout;
class Shader;

	}

    namespace world
    {

class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;

    }

    namespace shape
    {

/*! Solid entity.
 * \ingroup Shape
 */
class T_DLLCLASS SolidEntity : public world::Entity
{
    T_RTTI_CLASS;

public:
    SolidEntity(
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& shader,
		const Transform& transform
	);

    virtual void update(const world::UpdateParams& update) override final;

	void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

private:
	struct MaterialBatch
	{
		render::Primitives primitives;
	};

	Ref< render::IRenderSystem > m_renderSystem;
	resource::Proxy< render::Shader > m_shader;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	AlignedVector< MaterialBatch > m_batches;
	bool m_dirty;
};

    }
}