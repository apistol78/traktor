#include "Editor/DefaultPipeline.h"
#include "Editor/PipelineManager.h"
#include "Database/Instance.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.DefaultPipeline", DefaultPipeline, Pipeline)

bool DefaultPipeline::create(const Settings* settings)
{
	return true;
}

void DefaultPipeline::destroy()
{
}

uint32_t DefaultPipeline::getVersion() const
{
	return 1;
}

TypeSet DefaultPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Object >());
	return typeSet;
}

bool DefaultPipeline::buildDependencies(
	PipelineManager* pipelineManager,
	const Object* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	return true;
}

bool DefaultPipeline::buildOutput(
	PipelineManager* pipelineManager,
	const Object* sourceAsset,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< db::Instance > outputInstance = pipelineManager->createOutputInstance(outputPath, outputGuid, sourceAsset);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

	}
}
