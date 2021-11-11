#include "Editor/IPipelineDepends.h"
#include "Spark/Runtime/SparkLayerData.h"
#include "Spark/Runtime/Editor/SparkLayerPipeline.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SparkLayerPipeline", 0, SparkLayerPipeline, editor::DefaultPipeline)

TypeInfoSet SparkLayerPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< SparkLayerData >();
}

bool SparkLayerPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const SparkLayerData* layerData = mandatory_non_null_type_cast< const SparkLayerData* >(sourceAsset);

	pipelineDepends->addDependency(layerData->m_movie, editor::PdfBuild | editor::PdfResource);

	for (const auto& externalMovie : layerData->m_externalMovies)
		pipelineDepends->addDependency(externalMovie.second, editor::PdfBuild | editor::PdfResource);

	return true;
}

	}
}
