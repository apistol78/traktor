#include "Script/Editor/ScriptPipeline.h"
#include "Script/Script.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

Script* resolveScript(editor::IPipelineBuilder* pipelineBuilder, const Script* unresolvedScript)
{
	const std::vector< Guid >& dependencies = unresolvedScript->getDependencies();
	if (dependencies.empty())
	{
		return new Script(
			unresolvedScript->getText()
		);
	}

	StringOutputStream ss;
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		Ref< const Script > unresolvedDependency = pipelineBuilder->getObjectReadOnly< Script >(*i);
		if (!unresolvedDependency)
		{
			log::error << L"Script pipeline failed; unable to resolve dependency" << Endl;
			return 0;
		}

		Ref< Script > resolvedDependency = resolveScript(pipelineBuilder, unresolvedDependency);
		if (!resolvedDependency)
			return 0;

		ss << resolvedDependency->getText() << Endl << Endl;
	}

	ss << unresolvedScript->getText();

	return new Script(ss.str());
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptPipeline", 0, ScriptPipeline, editor::DefaultPipeline)

TypeInfoSet ScriptPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Script >());
	return typeSet;
}

bool ScriptPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const Script > sourceScript = checked_type_cast< const Script* >(sourceAsset);

	const std::vector< Guid >& dependencies = sourceScript->getDependencies();
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		pipelineDepends->addDependency(*i, false);

	return true;
}

bool ScriptPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const Script > sourceScript = checked_type_cast< const Script* >(sourceAsset);

	// Resolve script; ie. concate all dependent scripts.
	Ref< Script > outputScript = resolveScript(pipelineBuilder, sourceScript);
	if (!outputScript)
		return false;

	return DefaultPipeline::buildOutput(
		pipelineBuilder,
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
