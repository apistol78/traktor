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
#include "Core/Math/Vector4.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Resource/Proxy.h"
#include "Terrain/TerrainLayerComponent.h"
#include "Terrain/RubbleComponentData.h"

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::terrain
{

class RubbleComponent : public TerrainLayerComponent
{
	T_RTTI_CLASS;

public:
	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const RubbleComponentData& data
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
	struct RubbleMesh
	{
		resource::Proxy< mesh::InstanceMesh > mesh;
		uint8_t attribute;
		int32_t density;
		float randomScaleAmount;
		float randomTilt;
		float upness;
	};

	struct Instance
	{
		Vector4 position;
		Quaternion rotation;
		float scale;
	};

	struct Cluster
	{
		RubbleMesh* rubbleDef;
		Vector4 center;
		float distance;
		int32_t seed;
		int32_t from;
		int32_t to;
		bool visible;
	};

	world::Entity* m_owner = nullptr;
	RubbleComponentData m_data;
	AlignedVector< RubbleMesh > m_rubble;
	AlignedVector< Instance > m_instances;
	AlignedVector< Cluster > m_clusters;
	float m_clusterSize = 0.0f;
	Vector4 m_eye = Vector4::zero();
	Vector4 m_fwd = Vector4::zero();
	//AlignedVector< mesh::InstanceMesh::RenderInstance > m_instanceData;
};

}
