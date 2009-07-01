#include "Scene/Editor/ScenePipeline.h"
#include "Scene/SceneAsset.h"
#include "World/Entity/EntityInstance.h"
#include "Editor/PipelineManager.h"
#include "Database/Instance.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.scene.ScenePipeline", ScenePipeline, editor::IPipeline)

bool ScenePipeline::create(const editor::Settings* settings)
{
	return true;
}

void ScenePipeline::destroy()
{
}

uint32_t ScenePipeline::getVersion() const
{
	return 1;
}

TypeSet ScenePipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< SceneAsset >());
	return typeSet;
}

bool ScenePipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const SceneAsset > sceneAsset = checked_type_cast< const SceneAsset* >(sourceAsset);
	pipelineManager->addDependency(sceneAsset->getInstance());
	return true;
}

bool ScenePipeline::buildOutput(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< db::Instance > outputInstance = pipelineManager->createOutputInstance(outputPath, outputGuid);
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
