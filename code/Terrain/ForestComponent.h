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
#include "Core/Math/Aabb3.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Resource/Proxy.h"
#include "Terrain/ForestComponentData.h"
#include "Terrain/TerrainLayerComponent.h"

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

class ForestComponent : public TerrainLayerComponent
{
	T_RTTI_CLASS;

public:
	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const ForestComponentData& layerData
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
	struct Tree
	{
		Vector4 position;
		Quaternion rotation;
		float scale;
	};

	world::Entity* m_owner = nullptr;
	ForestComponentData m_data;
	resource::Proxy< mesh::InstanceMesh > m_lod0mesh;
	resource::Proxy< mesh::InstanceMesh > m_lod1mesh;
	resource::Proxy< mesh::InstanceMesh > m_lod2mesh;
	AlignedVector< Tree > m_trees;
	AlignedVector< uint32_t > m_lod0Indices;
	AlignedVector< uint32_t > m_lod1Indices;
	AlignedVector< uint32_t > m_lod2Indices;
	AlignedVector< uint32_t > m_lodShadowIndices;
	//AlignedVector< mesh::InstanceMesh::RenderInstance > m_instanceData;
	Aabb3 m_boundingBox;
};

}
