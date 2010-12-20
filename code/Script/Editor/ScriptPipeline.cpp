#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Script/IScriptManager.h"
#include "Script/IScriptResource.h"
#include "Script/Script.h"
#include "Script/Editor/ScriptPipeline.h"

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
			return 0;

		Ref< Script > resolvedDependency = resolveScript(pipelineBuilder, unresolvedDependency);
		if (!resolvedDependency)
			return 0;

		ss << resolvedDependency->getText() << Endl << Endl;
	}

	ss << unresolvedScript->getText();

	return new Script(ss.str());
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptPipeline", 2, ScriptPipeline, editor::DefaultPipeline)

bool ScriptPipeline::create(const editor::IPipelineSettings* settings)
{
	std::wstring scriptManagerTypeName = settings->getProperty< PropertyString >(L"Editor.ScriptManagerType");

	// Create script manager instance.
	const TypeInfo* scriptManagerType = TypeInfo::find(scriptManagerTypeName);
	if (!scriptManagerType)
	{
		log::error << L"Script pipeline failed; no such type \"" << scriptManagerTypeName << L"\"" << Endl;
		return false;
	}

	m_scriptManager = dynamic_type_cast< IScriptManager* >(scriptManagerType->createInstance());
	T_ASSERT (m_scriptManager);

	return editor::DefaultPipeline::create(settings);
}

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
		pipelineDepends->addDependency(*i, editor::PdfUse);

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
	Ref< Script > script = resolveScript(pipelineBuilder, sourceScript);
	if (!script)
	{
		log::error << L"Script pipeline failed; unable to resolve script dependencies" << Endl;
		return false;
	}

	// Compile script; save binary blobs if possible.
	Ref< IScriptResource > resource = m_scriptManager->compile(script->getText(), true, 0);
	if (!resource)
	{
		log::error << L"Script pipeline failed; unable to compile script" << Endl;
		return false;
	}

	return DefaultPipeline::buildOutput(
		pipelineBuilder,
		resource,
		sourceAssetHash,
		buildParams,
		outputPath,
		outputGuid,
		reason
	);
}

	}
}
