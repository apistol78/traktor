#ifndef traktor_terrain_UndergrowthLayerData_H
#define traktor_terrain_UndergrowthLayerData_H

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
	namespace terrain
	{

class T_DLLCLASS UndergrowthLayerData : public ITerrainLayerData
{
	T_RTTI_CLASS;

public:
	UndergrowthLayerData();

	virtual Ref< ITerrainLayer > createLayerInstance(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const TerrainEntity& terrainEntity
	) const override;

	virtual void serialize(ISerializer& s);

private:
	friend class UndergrowthLayer;
	friend class UndergrowthPipeline;

	resource::Id< render::Shader > m_shader;
	int32_t m_density;
	float m_spreadDistance;
	float m_cellRadius;
	float m_plantScale;
};

	}
}

#endif	// traktor_terrain_UndergrowthLayerData_H
