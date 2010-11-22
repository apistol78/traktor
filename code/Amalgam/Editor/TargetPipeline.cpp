#include "Amalgam/Editor/TargetPipeline.h"
#include "Amalgam/Editor/Target.h"
#include "Core/Serialization/ISerializable.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.TargetPipeline", 1, TargetPipeline, editor::IPipeline)

bool TargetPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void TargetPipeline::destroy()
{
}

TypeInfoSet TargetPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Target >());
	return typeSet;
}

bool TargetPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	return true;
}

bool TargetPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	return true;
}

	}
}
