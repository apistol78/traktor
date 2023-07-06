/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Math/TransformPath.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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

/*! Spline entity.
 * \ingroup Shape
 */
class T_DLLCLASS SplineComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit SplineComponent(
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& shader
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

	const TransformPath& getPath() const { return m_path; }

private:
	struct MaterialBatch
	{
		render::Primitives primitives;
	};

	Ref< render::IRenderSystem > m_renderSystem;
	resource::Proxy< render::Shader > m_shader;

	world::Entity* m_owner = nullptr;

	TransformPath m_path;
	bool m_dirty;

	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	AlignedVector< MaterialBatch > m_batches;
};

	}
}

