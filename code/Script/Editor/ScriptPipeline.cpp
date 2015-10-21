#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptResource.h"
#include "Script/Editor/Preprocessor.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptPipeline.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

struct ErrorCallback : public IErrorCallback
{
	virtual void syntaxError(const std::wstring& name, uint32_t line, const std::wstring& message)
	{
		log::error << name << L" (" << line << L"): " << message << Endl;
	}

	virtual void otherError(const std::wstring& message)
	{
		log::error << message << Endl;
	}
};

bool flattenDependencies(editor::IPipelineBuilder* pipelineBuilder, const Guid& scriptGuid, std::vector< Guid >& outScripts)
{
	Ref< db::Instance > scriptInstance = pipelineBuilder->getSourceDatabase()->getInstance(scriptGuid);
	if (!scriptInstance)
		return false;

	Ref< const Script > script = scriptInstance->getObject< Script >();
	if (!script)
		return false;

	const std::vector< Guid >& dependencies = script->getDependencies();
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (std::find(outScripts.begin(), outScripts.end(), *i) != outScripts.end())
			continue;

		if (!flattenDependencies(pipelineBuilder, *i, outScripts))
			return false;
	}

	outScripts.push_back(scriptGuid);
	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptPipeline", 20, ScriptPipeline, editor::DefaultPipeline)

ScriptPipeline::~ScriptPipeline()
{
	safeDestroy(m_scriptManager);
}

bool ScriptPipeline::create(const editor::IPipelineSettings* settings)
{
	// Get implementation type name; return true if no type specified to silence error from pipeline if scripting isn't used.
	std::wstring scriptManagerTypeName = settings->getProperty< PropertyString >(L"Editor.ScriptManagerType");
	if (scriptManagerTypeName.empty())
		return true;

	// Create script manager instance.
	const TypeInfo* scriptManagerType = TypeInfo::find(scriptManagerTypeName);
	if (!scriptManagerType)
	{
		log::error << L"Script pipeline failed; no such type \"" << scriptManagerTypeName << L"\"" << Endl;
		return false;
	}

	m_scriptManager = dynamic_type_cast< IScriptManager* >(scriptManagerType->createInstance());
	T_ASSERT (m_scriptManager);

	// Create preprocessor.
	m_preprocessor = new Preprocessor();
	
	std::set< std::wstring > definitions = settings->getProperty< PropertyStringSet >(L"ScriptPipeline.PreprocessorDefinitions");
	for (std::set< std::wstring >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
		m_preprocessor->setDefinition(*i);

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
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< const Script > sourceScript = checked_type_cast< const Script* >(sourceAsset);

	const std::vector< Guid >& dependencies = sourceScript->getDependencies();
	for (std::vector< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		pipelineDepends->addDependency(*i, editor::PdfBuild);

	return true;
}

bool ScriptPipeline::buildOutput(
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
	const Script* script = mandatory_non_null_type_cast< const Script* >(sourceAsset);

	// Ensure no double character line breaks.
	std::wstring source = script->getText();
	source = replaceAll< std::wstring >(source, L"\r\n", L"\n");

	// Execute preprocessor on script.
	std::wstring text;
	if (!m_preprocessor->evaluate(source, text))
	{
		log::error << L"Script pipeline failed; unable to preprocess script" << Endl;
		return false;
	}

	// Create ordered list of dependent scripts.
	std::vector< Guid > dependencies;
	if (!flattenDependencies(pipelineBuilder, outputGuid, dependencies))
	{
		log::error << L"Script pipeline failed; unable to resolve script dependencies" << Endl;
		return false;
	}

	// Ensure current script isn't part of dependencies.
	std::vector< Guid >::iterator i = std::find(dependencies.begin(), dependencies.end(), outputGuid);
	if (i != dependencies.end())
		dependencies.erase(i);

	// Compile script; save binary blobs if possible.
	ErrorCallback errorCallback;
	Ref< IScriptBlob > blob = m_scriptManager->compile(outputGuid.format(), text, &errorCallback);
	if (!blob)
	{
		log::error << L"Script pipeline failed; unable to compile script" << Endl;
		return false;
	}

	// Create output resource.
	Ref< ScriptResource > resource = new ScriptResource();
	resource->m_dependencies = dependencies;
	resource->m_blob = blob;

	return DefaultPipeline::buildOutput(
		pipelineBuilder,
		dependencySet,
		dependency,
		sourceInstance,
		resource,
		sourceAssetHash,
		outputPath,
		outputGuid,
		buildParams,
		reason
	);
}

	}
}
