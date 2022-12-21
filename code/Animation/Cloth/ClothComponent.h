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
#include "Core/Math/Vector2.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;

}

namespace traktor::animation
{

/*!
 * \ingroup Animation
 */
class T_DLLCLASS ClothComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	struct Node
	{
		Vector4 position[2];
		Vector2 texCoord;
		Scalar invMass;
	};

	struct Edge
	{
		uint32_t index[2];
		Scalar length;
	};

	ClothComponent();

	virtual ~ClothComponent();

	bool create(
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& shader,
		uint32_t resolutionX,
		uint32_t resolutionY,
		float scale,
		float damping,
		uint32_t solverIterations
	);

	void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void reset();

	void setNodeInvMass(uint32_t x, uint32_t y, float invMass);

	const AlignedVector< Node >& getNodes() const { return m_nodes; }

	const AlignedVector< Edge >& getEdges() const { return m_edges; }

private:
	AlignedVector< Node > m_nodes;
	AlignedVector< Edge > m_edges;
	Transform m_transform;
	float m_time;
	float m_updateTime;
	float m_scale;
	Scalar m_damping;
	uint32_t m_solverIterations;
	uint32_t m_resolutionX;
	uint32_t m_resolutionY;
	uint32_t m_triangleCount;
	traktor::Aabb3 m_aabb;
	bool m_updateRequired;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	resource::Proxy< render::Shader > m_shader;
};

}
