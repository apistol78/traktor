#include "Weather/Editor/WeatherEntityPipeline.h"
#include "Weather/Sky/SkyEntityData.h"
#include "Editor/PipelineManager.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.weather.WeatherEntityPipeline", WeatherEntityPipeline, world::EntityPipeline)

TypeSet WeatherEntityPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< SkyEntityData >());
	return typeSet;
}

bool WeatherEntityPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const SkyEntityData* skyEntityData = dynamic_type_cast< const SkyEntityData* >(sourceAsset))
		pipelineManager->addDependency(skyEntityData->getShader().getGuid());
	return true;
}

	}
}
