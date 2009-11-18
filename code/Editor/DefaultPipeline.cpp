#include "Editor/DefaultPipeline.h"
#include "Editor/IPipelineBuilder.h"
#include "Database/Instance.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.DefaultPipeline", DefaultPipeline, IPipeline)

bool DefaultPipeline::create(const IPipelineSettings* settings)
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

TypeInfoSet DefaultPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Object >());
	return typeSet;
}

bool DefaultPipeline::buildDependencies(
	IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	return true;
}

bool DefaultPipeline::buildOutput(
	IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(sourceAsset);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

	}
}
