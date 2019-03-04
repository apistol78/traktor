#include "Editor/IPipelineDepends.h"
#include "Flash/Runtime/FlashLayerData.h"
#include "Flash/Runtime/Editor/FlashLayerPipeline.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashLayerPipeline", 0, FlashLayerPipeline, editor::DefaultPipeline)

TypeInfoSet FlashLayerPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< FlashLayerData >();
}

bool FlashLayerPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const FlashLayerData* layerData = mandatory_non_null_type_cast< const FlashLayerData* >(sourceAsset);

	pipelineDepends->addDependency(layerData->m_movie, editor::PdfBuild);

	for (const auto& externalMovie : layerData->m_externalMovies)
		pipelineDepends->addDependency(externalMovie.second, editor::PdfBuild);

	pipelineDepends->addDependency(layerData->m_imageProcess, editor::PdfBuild);

	return true;
}

	}
}