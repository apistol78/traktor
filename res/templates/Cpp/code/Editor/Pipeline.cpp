#include <Core/Serialization/DeepClone.h>
#include <Editor/IPipelineDepends.h>
#include "Editor/Pipeline.h"
#include "Shared/QuadComponentData.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Pipeline", 0, Pipeline, editor::IPipeline)

bool Pipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void Pipeline::destroy()
{
}

TypeInfoSet Pipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< QuadComponentData >());
	return typeSet;
}

bool Pipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const QuadComponentData* quadComponentData = mandatory_non_null_type_cast< const QuadComponentData* >(sourceAsset);
	pipelineDepends->addDependency(quadComponentData->m_shader, editor::PdfBuild | editor::PdfResource);
	return true;
}

bool Pipeline::buildOutput(
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
	return false;
}

Ref< ISerializable > Pipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	Ref< QuadComponentData > outputData = DeepClone(sourceAsset).create< QuadComponentData >();
	return outputData;
}
