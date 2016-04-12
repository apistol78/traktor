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

bool flattenDependencies(editor::IPipelineBuilder* pipelineBuilder, const Preprocessor* prep, const Guid& scriptGuid, std::vector< Guid >& outScripts)
{
	Guid g;

	Ref< db::Instance > scriptInstance = pipelineBuilder->getSourceDatabase()->getInstance(scriptGuid);
	if (!scriptInstance)
		return false;

	Ref< const Script > script = scriptInstance->getObject< Script >();
	if (!script)
		return false;

	// Escape script and flatten dependencies.
	std::wstring source = script->escape([&] (const Guid& g) -> std::wstring {
		return g.format();
	});

	// Ensure no double character line breaks.
	source = replaceAll< std::wstring >(source, L"\r\n", L"\n");

	// Execute preprocessor on script.
	std::wstring text;
	std::set< std::wstring > usings;
	if (!prep->evaluate(source, text, usings))
	{
		log::error << L"Script pipeline failed; unable to preprocess script." << Endl;
		return false;
	}

	// Scan usings.
	for (std::set< std::wstring >::const_iterator i = usings.begin(); i != usings.end(); ++i)
	{
		if (g.create(*i))
		{
			if (std::find(outScripts.begin(), outScripts.end(), g) != outScripts.end())
				continue;
			if (!flattenDependencies(pipelineBuilder, prep, g, outScripts))
				return false;
		}
	}

	outScripts.push_back(scriptGuid);
	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptPipeline", 21, ScriptPipeline, editor::DefaultPipeline)

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
	Ref< const Script > script = checked_type_cast< const Script* >(sourceAsset);

	// Escape script and flatten dependencies.
	std::wstring source = script->escape([&] (const Guid& g) -> std::wstring {
		return g.format();
	});

	// Ensure no double character line breaks.
	source = replaceAll< std::wstring >(source, L"\r\n", L"\n");

	// Execute preprocessor on script.
	std::wstring text;
	std::set< std::wstring > usings;
	if (!m_preprocessor->evaluate(source, text, usings))
	{
		log::error << L"Script pipeline failed; unable to preprocess script." << Endl;
		return false;
	}

	// Get list of used dependencies.
	std::set< Guid > usingIds;
	for (std::set< std::wstring >::const_iterator i = usings.begin(); i != usings.end(); ++i)
	{
		Guid g;
		if (g.create(*i))
			usingIds.insert(g);
		else
		{
			log::error << L"Script pipeline failed; malformed using statement." << Endl;
			return false;
		}
	}

	// Add dependencies to included scripts.
	for (std::set< Guid >::const_iterator i = usingIds.begin(); i != usingIds.end(); ++i)
		pipelineDepends->addDependency(*i, editor::PdfBuild);

	// Scan for implicit dependencies from script.
	for (size_t i = text.find_first_of(L"{"); i != text.npos; i = text.find(L"{", i + 1))
	{
		if (i + 37 > text.size())
			break;

		if (text[i + 37] != L'}')
			continue;

		Guid g;
		if (!g.create(text.substr(i, 38)))
			continue;

		if (usingIds.find(g) != usingIds.end())
			continue;

		if (pipelineDepends->getSourceDatabase()->getInstance(g))
			pipelineDepends->addDependency(g, editor::PdfBuild);
	}

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

	// Escape script and flatten dependencies.
	std::wstring source = script->escape([&] (const Guid& g) -> std::wstring {
		return g.format();
	});

	// Ensure no double character line breaks.
	source = replaceAll< std::wstring >(source, L"\r\n", L"\n");

	// Execute preprocessor on script.
	std::wstring text;
	std::set< std::wstring > includes;
	if (!m_preprocessor->evaluate(source, text, includes))
	{
		log::error << L"Script pipeline failed; unable to preprocess script." << Endl;
		return false;
	}

	// Create ordered list of dependent scripts.
	std::vector< Guid > dependencies;
	if (!flattenDependencies(pipelineBuilder, m_preprocessor, outputGuid, dependencies))
	{
		log::error << L"Script pipeline failed; unable to resolve script dependencies." << Endl;
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
		log::error << L"Script pipeline failed; unable to compile script." << Endl;
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
