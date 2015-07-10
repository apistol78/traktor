#ifndef traktor_terrain_UndergrowthLayer_H
#define traktor_terrain_UndergrowthLayer_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Resource/Proxy.h"
#include "Terrain/ITerrainLayer.h"
#include "Terrain/UndergrowthLayerData.h"

namespace traktor
{
	namespace render
	{

class IndexBuffer;
class IRenderSystem;
class Shader;
class VertexBuffer;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace terrain
	{

class UndergrowthLayer : public ITerrainLayer
{
	T_RTTI_CLASS;

public:
	UndergrowthLayer();

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const UndergrowthLayerData& layerData,
		const TerrainEntity& terrainEntity
	);

	virtual void update(const world::UpdateParams& update) T_OVERRIDE;

	virtual void render(
		TerrainEntity& terrainEntity,
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) T_OVERRIDE;

	virtual void updatePatches(const TerrainEntity& terrainEntity) T_OVERRIDE;

private:
	struct Cluster
	{
		Vector4 center;
		float distance;
		uint8_t plant;
		float plantScale;
		bool visible;
		int32_t from;
		int32_t to;
	};

	UndergrowthLayerData m_layerData;

	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	resource::Proxy< render::Shader > m_shader;
	AlignedVector< Vector4 > m_plants;
	AlignedVector< Cluster > m_clusters;
	float m_clusterSize;
	uint32_t m_count;
};

	}
}

#endif	// traktor_terrain_UndergrowthLayer_H
