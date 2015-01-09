#ifndef traktor_terrain_RubbleLayer_H
#define traktor_terrain_RubbleLayer_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Resource/Proxy.h"
#include "Terrain/ITerrainLayer.h"
#include "Terrain/RubbleLayerData.h"

namespace traktor
{
	namespace render
	{

//class IndexBuffer;
class IRenderSystem;
//class Shader;
//class VertexBuffer;

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
	struct Instance
	{
		Vector4 position;
		Quaternion rotation;
		float scale;
	};

	struct Cluster
	{
		Vector4 center;
		float distance;
		bool visible;
		float scale;
		int32_t from;
		int32_t to;
	};

	RubbleLayerData m_layerData;

	resource::Proxy< mesh::InstanceMesh > m_mesh;
	AlignedVector< Instance > m_instances;
	AlignedVector< Cluster > m_clusters;
	float m_clusterSize;
};

	}
}

#endif	// traktor_terrain_RubbleLayer_H
