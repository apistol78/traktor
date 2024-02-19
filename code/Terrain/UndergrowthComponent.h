/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Resource/Proxy.h"
#include "Terrain/TerrainLayerComponent.h"
#include "Terrain/UndergrowthComponentData.h"

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class IVertexLayout;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::terrain
{

class UndergrowthComponent : public TerrainLayerComponent
{
	T_RTTI_CLASS;

public:
	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const UndergrowthComponentData& layerData
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) override final;

	virtual void updatePatches() override final;

private:
	struct Cluster
	{
		Vector4 center;
		uint8_t plant;
		float plantScale;
		int32_t from;
		int32_t to;
	};

	struct ViewState
	{
		Ref< render::Buffer > plantBuffer;
		Ref< render::Buffer > orderBuffer;
		int32_t drawInstanceCount;
	};

	world::Entity* m_owner = nullptr;
	UndergrowthComponentData m_layerData;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	resource::Proxy< render::Shader > m_shader;
	AlignedVector< Cluster > m_clusters;
	SmallMap< int32_t, ViewState > m_viewState;
	float m_clusterSize = 0.0f;
	uint32_t m_plantsCount = 0;
};

}
