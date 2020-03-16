#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/BitVector.h"
#include "Core/Math/Vector4.h"
#include "Resource/Proxy.h"
#include "Terrain/TerrainLayerComponent.h"
#include "Terrain/UndergrowthComponentData.h"

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

class UndergrowthComponent : public TerrainLayerComponent
{
	T_RTTI_CLASS;

public:
	UndergrowthComponent();

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const UndergrowthComponentData& layerData
	);

	virtual void destroy() override final;

	virtual void setOwner(world::ComponentEntity* owner) override final;

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
		AlignedVector< Vector4 > plants;
		AlignedVector< float > distances;
		BitVector pvs;
		uint32_t count;
	};

	world::ComponentEntity* m_owner;

	UndergrowthComponentData m_layerData;

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

