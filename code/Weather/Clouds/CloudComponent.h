/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"
#include "Weather/Clouds/CloudParticleCluster.h"
#include "Weather/Clouds/CloudParticleData.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class IRenderTargetSet;
class ITexture;
class IVertexLayout;
class RenderContext;
class PrimitiveRenderer;

}

namespace traktor::world
{

class IWorldRenderPass;
class WorldRenderView;

}

namespace traktor::weather
{

class CloudMask;

class T_DLLCLASS CloudComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	CloudComponent();

	virtual ~CloudComponent();

	bool create(
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& particleShader,
		const resource::Proxy< render::ITexture >& particleTexture,
		const resource::Proxy< render::Shader >& impostorShader,
		const resource::Proxy< CloudMask >& mask,
		uint32_t impostorTargetResolution,
		uint32_t impostorSliceCount,
		uint32_t updateFrequency,
		float updatePositionThreshold,
		float updateDirectionThreshold,
		const CloudParticleData& particleData
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void build(
		render::RenderContext* renderContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		render::PrimitiveRenderer* primitiveRenderer
	);

private:
	world::Entity* m_owner;
	resource::Proxy< render::Shader > m_particleShader;
	resource::Proxy< render::ITexture > m_particleTexture;
	resource::Proxy< render::Shader > m_impostorShader;
	resource::Proxy< CloudMask > m_mask;
	RefArray< render::IRenderTargetSet > m_impostorTargets;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	render::handle_t m_handleBillboardView;
	render::handle_t m_handleImpostorTarget;
	uint32_t m_impostorSliceCount;
	uint32_t m_updateFrequency;
	float m_updatePositionThreshold;
	float m_updateDirectionThreshold;
	CloudParticleCluster m_cluster;
	CloudParticleData m_particleData;
	Vector4 m_lastCameraPosition;
	Vector4 m_lastCameraDirection;
	float m_timeUntilUpdate;
	uint32_t m_updateCount;

	void buildCluster(
		render::RenderContext* renderContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		render::PrimitiveRenderer* primitiveRenderer,
		const CloudParticleCluster& cluster
	);
};

}
