#ifndef traktor_mesh_WeatherEntityPipeline_H
#define traktor_mesh_WeatherEntityPipeline_H

#include "World/Editor/EntityPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace weather
	{

class T_DLLCLASS WeatherEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS(WeatherEntityPipeline)

public:
	virtual TypeSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::PipelineManager* pipelineManager,
		const Object* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;
};

	}
}

#endif	// traktor_mesh_WeatherEntityPipeline_H
