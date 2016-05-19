#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StringReader.h"
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
#include "Script/Editor/ScriptAsset.h"
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

bool readScript(editor::IPipelineCommon* pipelineCommon, const std::wstring& assetPath, const ISerializable* sourceAsset, std::wstring& outSource)
{
	if (const Script* script = dynamic_type_cast< const Script* >(sourceAsset))
	{
		// Escape script and flatten dependencies.
		outSource = script->escape([&] (const Guid& g) -> std::wstring {
			return g.format();
		});
	}
	else if (const ScriptAsset* scriptAsset = dynamic_type_cast< const ScriptAsset* >(sourceAsset))
	{
		// Read script from asset as-is as we need to traverse dependencies.
		Ref< IStream > file = pipelineCommon->openFile(Path(assetPath), scriptAsset->getFileName().getOriginal());
		if (!file)
		{
			log::error << L"Script pipeline failed; unable to open script (" << scriptAsset->getFileName().getOriginal() << L")" << Endl;
			return false;
		}

		StringOutputStream ss;
		std::wstring line;

		// Read script using utf-8 encoding.
		Utf8Encoding encoding;
		StringReader sr(file, &encoding);
		while (sr.readLine(line) >= 0)
			ss << line << Endl;

		outSource = ss.str();
	}

	// Ensure no double character line breaks.
	outSource = replaceAll< std::wstring >(outSource, L"\r\n", L"\n");
	return true;
}

bool flattenDependencies(editor::IPipelineBuilder* pipelineBuilder, const std::wstring& assetPath, const Preprocessor* prep, const Guid& scriptGuid, std::vector< Guid >& outScripts)
{
	Guid g;

	Ref< db::Instance > scriptInstance = pipelineBuilder->getSourceDatabase()->getInstance(scriptGuid);
	if (!scriptInstance)
		return false;

	std::wstring source;
	if (!readScript(pipelineBuilder, assetPath, scriptInstance->getObject(), source))
		return false;

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
		if (!g.create(*i))
		{
			// Resolve using database path instead of guid.
			Ref< db::Instance > dependentInstance = pipelineBuilder->getSourceDatabase()->getInstance(*i);
			if (!dependentInstance)
				return false;

			g = dependentInstance->getGuid();
		}

		if (std::find(outScripts.begin(), outScripts.end(), g) != outScripts.end())
			continue;

		if (!flattenDependencies(pipelineBuilder, assetPath, prep, g, outScripts))
			return false;
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

	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");

	return editor::DefaultPipeline::create(settings);
}

TypeInfoSet ScriptPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Script >());
	typeSet.insert(&type_of< ScriptAsset >());
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
	// Add dependency to script file.
	if (const ScriptAsset* scriptAsset = dynamic_type_cast< const ScriptAsset* >(sourceAsset))
		pipelineDepends->addDependency(Path(m_assetPath), scriptAsset->getFileName().getOriginal());

	std::wstring source;
	if (!readScript(pipelineDepends, m_assetPath, sourceAsset, source))
		return false;

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
			// Resolve using database path instead of guid.
			Ref< db::Instance > dependentInstance = pipelineDepends->getSourceDatabase()->getInstance(*i);
			if (dependentInstance)
				usingIds.insert(dependentInstance->getGuid());
			else
			{
				log::error << L"Script pipeline failed; malformed using statement." << Endl;
				return false;
			}
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
	std::wstring source;
	if (!readScript(pipelineBuilder, m_assetPath, sourceAsset, source))
		return false;

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
	if (!flattenDependencies(pipelineBuilder, m_assetPath, m_preprocessor, outputGuid, dependencies))
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
