#include "Script/Editor/ScriptPipeline.h"
#include "Script/Script.h"
#include "Editor/IPipelineManager.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

Script* resolveScript(editor::IPipelineManager* pipelineManager, const Script* unresolvedScript)
{
	const std::vector< Guid >& dependencies = unresolvedScript->getDependencies();
	if (dependencies.empty())
	{
		return gc_new< Script >(
			unresolvedScript->getText()
		);
	}

	StringOutputStream ss;
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		Ref< const Script > unresolvedDependency = pipelineManager->getObjectReadOnly< Script >(*i);
		if (!unresolvedDependency)
		{
			log::error << L"Script pipeline failed; unable to resolve dependency" << Endl;
			return 0;
		}

		Ref< Script > resolvedDependency = resolveScript(pipelineManager, unresolvedDependency);
		if (!resolvedDependency)
			return 0;

		ss << resolvedDependency->getText() << Endl << Endl;
	}

	ss << unresolvedScript->getText();

	return gc_new< Script >(ss.str());
}

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.script.ScriptPipeline", ScriptPipeline, editor::DefaultPipeline)

TypeSet ScriptPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Script >());
	return typeSet;
}

bool ScriptPipeline::buildDependencies(
	editor::IPipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const Script > sourceScript = checked_type_cast< const Script* >(sourceAsset);

	const std::vector< Guid >& dependencies = sourceScript->getDependencies();
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		pipelineManager->addDependency(*i, false);

	return true;
}

bool ScriptPipeline::buildOutput(
	editor::IPipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const Script > sourceScript = checked_type_cast< const Script* >(sourceAsset);

	// Resolve script; ie. concate all dependent scripts.
	Ref< Script > outputScript = resolveScript(pipelineManager, sourceScript);
	if (!outputScript)
		return false;

	return DefaultPipeline::buildOutput(
		pipelineManager,
		outputScript,
		sourceAssetHash,
		buildParams,
		outputPath,
		outputGuid,
		reason
	);
}

	}
}
