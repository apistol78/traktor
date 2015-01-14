#ifndef traktor_terrain_RubbleLayer_H
#define traktor_terrain_RubbleLayer_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Resource/Proxy.h"
#include "Terrain/ITerrainLayer.h"
#include "Terrain/RubbleLayerData.h"

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

class RubbleLayer : public ITerrainLayer
{
	T_RTTI_CLASS;

public:
	RubbleLayer();

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const RubbleLayerData& layerData,
		const TerrainEntity& terrainEntity
	);

	virtual void update(const world::UpdateParams& update) override;

	virtual void render(
		TerrainEntity& terrainEntity,
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) override;

	virtual void updatePatches(const TerrainEntity& terrainEntity) override;

private:
	struct RubbleMesh
	{
		resource::Proxy< mesh::InstanceMesh > mesh;
		uint8_t material;
		int32_t density;
		float randomScaleAmount;
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
		bool visible;
		float scale;
		int32_t from;
		int32_t to;
	};

	std::vector< RubbleMesh > m_rubble;
	AlignedVector< Instance > m_instances;
	AlignedVector< Cluster > m_clusters;
	float m_spreadDistance;
	float m_clusterSize;
	Vector4 m_eye;
	AlignedVector< mesh::InstanceMesh::instance_distance_t > m_instanceData;
};

	}
}

#endif	// traktor_terrain_RubbleLayer_H
