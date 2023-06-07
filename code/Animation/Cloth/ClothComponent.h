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

namespace traktor
{

class Job;

}

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
		render::handle_t jointName;
		Vector4 jointOffset;
		Vector4 position[2];
		Vector2 texCoord;
		Scalar invMass;
	};

	struct Edge
	{
		uint32_t index[2];
		Scalar length;
	};

	bool create(
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& shader,
		uint32_t resolutionX,
		uint32_t resolutionY,
		float scale,
		float jointRadius,
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

	void setNodeAnchor(render::handle_t jointName, const Vector4& jointOffset, uint32_t x, uint32_t y);

	const AlignedVector< Node >& getNodes() const { return m_nodes; }

	const AlignedVector< Edge >& getEdges() const { return m_edges; }

private:
	world::Entity* m_owner = nullptr;
	AlignedVector< Node > m_nodes;
	AlignedVector< Edge > m_edges;
	Transform m_transform;
	float m_time = 4.0f;
	float m_updateTime = 0.0f;
	float m_scale = 1.0f;
	Scalar m_damping = 0.1_simd;
	Scalar m_jointRadius = 0.1_simd;
	uint32_t m_solverIterations = 4;
	uint32_t m_resolutionX = 10;
	uint32_t m_resolutionY = 10;
	uint32_t m_triangleCount = 0;
	traktor::Aabb3 m_aabb;
	bool m_updateRequired = true;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	resource::Proxy< render::Shader > m_shader;
	Vector4 m_lastPosition = Vector4::zero();
	mutable Ref< Job > m_updateClothJob;

	void synchronize() const;
};

}
