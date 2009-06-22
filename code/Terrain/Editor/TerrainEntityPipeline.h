#ifndef traktor_terrain_TerrainEntityPipeline_H
#define traktor_terrain_TerrainEntityPipeline_H

#include "World/Editor/EntityPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

class T_DLLCLASS TerrainEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS(TerrainEntityPipeline)

public:
	virtual TypeSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::PipelineManager* pipelineManager,
		const Serializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;
};
	}
}

#endif	// traktor_terrain_TerrainEntityPipeline_H
