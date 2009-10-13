#include "Weather/Editor/WeatherEntityPipeline.h"
#include "Weather/Sky/SkyEntityData.h"
#include "Weather/Clouds/CloudEntityData.h"
#include "Editor/IPipelineManager.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.weather.WeatherEntityPipeline", WeatherEntityPipeline, world::EntityPipeline)

TypeSet WeatherEntityPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< SkyEntityData >());
	typeSet.insert(&type_of< CloudEntityData >());
	return typeSet;
}

bool WeatherEntityPipeline::buildDependencies(
	editor::IPipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const SkyEntityData* skyEntityData = dynamic_type_cast< const SkyEntityData* >(sourceAsset))
		pipelineManager->addDependency(skyEntityData->getShader().getGuid(), true);
	if (const CloudEntityData* cloudEntityData = dynamic_type_cast< const CloudEntityData* >(sourceAsset))
	{
		pipelineManager->addDependency(cloudEntityData->getParticleShader().getGuid(), true);
		pipelineManager->addDependency(cloudEntityData->getImpostorShader().getGuid(), true);
	}
	return true;
}

	}
}
