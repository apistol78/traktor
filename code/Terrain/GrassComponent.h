/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Vector4.h"
#include "Resource/Proxy.h"
#include "Terrain/GrassComponentData.h"
#include "Terrain/TerrainLayerComponent.h"

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

class TerrainComponent;

/*! Grass layer component.
 * Renders segmented blade meshes, oriented and animated by the shader, scattered on terrain near camera.
 */
class GrassComponent : public TerrainLayerComponent
{
	T_RTTI_CLASS;

public:
	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const GrassComponentData& layerData);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual void setup(const world::WorldRenderView& worldRenderView) override final;

	virtual void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass) override final;

	virtual void updatePatches() override final;

private:
	//! Blades of a single grass type in a cluster near camera.
	struct Cluster
	{
		Vector4 center;
		uint32_t revision = 0;			//!< Revision of terrain tile when scattered.
		AlignedVector< float > blades;	//!< Eight floats per blade.
		int32_t from = 0;				//!< First blade in blade buffer.
		int32_t to = 0;
	};

	struct ViewState
	{
		Ref< render::Buffer > orderBuffer;
		int32_t drawInstanceCount = 0;
	};

	world::Entity* m_owner = nullptr;
	GrassComponentData m_layerData;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	Ref< render::Buffer > m_bladeBuffer;
	resource::Proxy< render::Shader > m_shader;
	SmallMap< uint64_t, Cluster > m_clusters;	//!< Keyed by cluster coordinate and grass type.
	AlignedVector< uint32_t > m_tileRevisions;
	SmallMap< int32_t, ViewState > m_viewState;
	int32_t m_eyeCluster[2] = { 0, 0 };
	bool m_valid = false;	//!< Clusters are scattered around eye cluster.
	float m_clusterSize = 0.0f;
	uint32_t m_bladesCount = 0;

	//! Scatter clusters within spread distance of eye, reuse already scattered clusters, and fill blade buffer.
	void updateClusters(const TerrainComponent* terrainComponent, const Vector4& eye, bool force);
};

}
