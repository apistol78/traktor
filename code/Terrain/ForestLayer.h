#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Resource/Proxy.h"
#include "Terrain/ForestLayerData.h"
#include "Terrain/ITerrainLayer.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace terrain
	{

class ForestLayer : public ITerrainLayer
{
	T_RTTI_CLASS;

public:
	ForestLayer();

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const ForestLayerData& layerData,
		const TerrainComponent& terrainComponent
	);

	virtual void update(const world::UpdateParams& update) override final;

	virtual void build(
		TerrainComponent& terrainComponent,
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) override final;

	virtual void updatePatches(const TerrainComponent& terrainComponent) override final;

private:
	struct Tree
	{
		Vector4 position;
		Quaternion rotation;
		float scale;
	};

	ForestLayerData m_data;

	resource::Proxy< mesh::InstanceMesh > m_lod0mesh;
	resource::Proxy< mesh::InstanceMesh > m_lod1mesh;

	AlignedVector< Tree > m_trees;
	AlignedVector< uint32_t > m_lod0indices;
	AlignedVector< uint32_t > m_lod1indices;

	AlignedVector< mesh::InstanceMesh::RenderInstance > m_instanceData;
};

	}
}

