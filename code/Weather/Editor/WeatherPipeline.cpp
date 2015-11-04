#include "Editor/IPipelineDepends.h"
#include "Weather/Clouds/CloudComponentData.h"
#include "Weather/Editor/WeatherPipeline.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.weather.WeatherPipeline", 0, WeatherPipeline, world::EntityPipeline)

TypeInfoSet WeatherPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SkyComponentData >());
	typeSet.insert(&type_of< CloudComponentData >());
	return typeSet;
}

bool WeatherPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const SkyComponentData* skyComponentData = dynamic_type_cast< const SkyComponentData* >(sourceAsset))
		pipelineDepends->addDependency(skyComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
	if (const CloudComponentData* cloudComponentData = dynamic_type_cast< const CloudComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(cloudComponentData->getParticleShader(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(cloudComponentData->getParticleTexture(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(cloudComponentData->getImpostorShader(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(cloudComponentData->getMask(), editor::PdfBuild | editor::PdfResource);
	}
	return true;
}

	}
}
