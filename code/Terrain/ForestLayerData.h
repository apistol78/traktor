#pragma once

#include "Core/Containers/AlignedVector.h"
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

class T_DLLCLASS ForestLayerData : public ITerrainLayerData
{
	T_RTTI_CLASS;

public:
	ForestLayerData();

	virtual Ref< ITerrainLayer > createLayerInstance(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const TerrainComponent& terrainComponent
	) const override;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ForestLayer;
	friend class TerrainEntityPipeline;

	resource::Id< mesh::InstanceMesh > m_lod0mesh;
	resource::Id< mesh::InstanceMesh > m_lod1mesh;

	uint8_t m_material;
	float m_density;
	float m_lod0distance;
	float m_lod1distance;
	float m_upness;		//!< Upness of tree, 0 means follow normal of terrain and 1 means completely straight up.
	float m_randomScale;
	float m_randomTilt;
	float m_slopeAngleThreshold;
};

	}
}
