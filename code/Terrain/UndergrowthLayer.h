#ifndef traktor_terrain_UndergrowthLayer_H
#define traktor_terrain_UndergrowthLayer_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/BitVector.h"
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
		const TerrainComponent& terrainEntity
	);

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void render(
		TerrainComponent& terrainComponent,
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) T_OVERRIDE T_FINAL;

	virtual void updatePatches(const TerrainComponent& terrainComponent) T_OVERRIDE T_FINAL;

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
		AlignedVector< Vector4 > plants;
		AlignedVector< float > distances;
		BitVector pvs;
		uint32_t count;
	};

	UndergrowthLayerData m_layerData;

	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	resource::Proxy< render::Shader > m_shader;
	AlignedVector< Cluster > m_clusters;
	SmallMap< int32_t, ViewState > m_viewState;
	float m_clusterSize;
	uint32_t m_plantsCount;
};

	}
}

#endif	// traktor_terrain_UndergrowthLayer_H
