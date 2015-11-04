#include "Editor/IPipelineDepends.h"
#include "Weather/Clouds/CloudEntityData.h"
#include "Weather/Editor/WeatherEntityPipeline.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.weather.WeatherEntityPipeline", 0, WeatherEntityPipeline, world::EntityPipeline)

TypeInfoSet WeatherEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SkyComponentData >());
	typeSet.insert(&type_of< CloudEntityData >());
	return typeSet;
}

bool WeatherEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const SkyComponentData* skyComponentData = dynamic_type_cast< const SkyComponentData* >(sourceAsset))
		pipelineDepends->addDependency(skyComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
	if (const CloudEntityData* cloudEntityData = dynamic_type_cast< const CloudEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(cloudEntityData->getParticleShader(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(cloudEntityData->getParticleTexture(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(cloudEntityData->getImpostorShader(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(cloudEntityData->getMask(), editor::PdfBuild | editor::PdfResource);
	}
	return true;
}

	}
}
