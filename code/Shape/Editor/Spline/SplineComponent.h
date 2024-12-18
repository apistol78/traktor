/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
#include "World/Entity/RTWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

}

namespace traktor::model
{

class Model;

}

namespace traktor::physics
{

class Body;
class PhysicsManager;

}

namespace traktor::render
{

class Buffer;
class IAccelerationStructure;
class IRenderSystem;
class IVertexLayout;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class IWorldRenderPass;
class World;
class WorldBuildContext;
class WorldRenderView;

}

namespace traktor::shape
{

class SplineComponentData;

/*! Spline entity.
 * \ingroup Shape
 */
class T_DLLCLASS SplineComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit SplineComponent(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		physics::PhysicsManager* physicsManager,
		const resource::Proxy< render::Shader >& defaultShader,
		const SplineComponentData* data
	);

	virtual ~SplineComponent();

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setWorld(world::World* world) override final;

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
		resource::Proxy< render::Shader > shader;
		render::Primitives primitives;
	};

	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< physics::PhysicsManager > m_physicsManager;

	resource::Proxy< render::Shader > m_defaultShader;

	Ref< const SplineComponentData > m_data;
	world::Entity* m_owner = nullptr;
	world::World* m_world = nullptr;

	TransformPath m_path;
	bool m_dirty;

	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	AlignedVector< MaterialBatch > m_batches;
	world::RTWorldComponent::Instance* m_rtwInstance = nullptr;

	Ref< physics::Body > m_body;

	Ref< Job > m_updateJob;
	Ref< model::Model > m_updateJobModel;
};

}

