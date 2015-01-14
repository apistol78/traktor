#ifndef traktor_terrain_RubbleLayerData_H
#define traktor_terrain_RubbleLayerData_H

#include "Resource/Id.h"
#include "Terrain/ITerrainLayerData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class InstanceMesh;

	}

	namespace terrain
	{

class T_DLLCLASS RubbleLayerData : public ITerrainLayerData
{
	T_RTTI_CLASS;

public:
	RubbleLayerData();

	virtual Ref< ITerrainLayer > createLayerInstance(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const TerrainEntity& terrainEntity
	) const override;

	virtual void serialize(ISerializer& s);

private:
	friend class RubbleLayer;
	friend class TerrainLayerPipeline;

	struct RubbleMesh
	{
		resource::Id< mesh::InstanceMesh > mesh;
		uint8_t material;
		int32_t density;
		float randomScaleAmount;

		RubbleMesh();

		void serialize(ISerializer& s);
	};

	float m_spreadDistance;
	std::vector< RubbleMesh > m_rubble;
};

	}
}

#endif	// traktor_terrain_RubbleLayerData_H
