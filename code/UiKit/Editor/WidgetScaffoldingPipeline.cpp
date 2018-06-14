#include "Editor/IPipelineDepends.h"
#include "UiKit/Editor/WidgetScaffolding.h"
#include "UiKit/Editor/WidgetScaffoldingPipeline.h"

namespace traktor
{
	namespace uikit
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.uikit.WidgetScaffoldingPipeline", 0, WidgetScaffoldingPipeline, editor::IPipeline)

bool WidgetScaffoldingPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void WidgetScaffoldingPipeline::destroy()
{
}

TypeInfoSet WidgetScaffoldingPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< WidgetScaffolding >();
}

bool WidgetScaffoldingPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const WidgetScaffolding* ws = mandatory_non_null_type_cast< const WidgetScaffolding* >(sourceAsset);
	pipelineDepends->addDependency(ws->getScaffoldingClass(), editor::PdfBuild);
	return true;
}

bool WidgetScaffoldingPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	return true;
}

Ref< ISerializable > WidgetScaffoldingPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	return 0;
}

	}
}
