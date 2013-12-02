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
#include "Script/IScriptResource.h"
#include "Script/Editor/Preprocessor.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptPipeline.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

struct ResolvedScript
{
	Guid id;
	std::wstring name;
	Ref< const Script > script;

	bool operator == (const Guid& rh) const { return id == rh; }
};

bool resolveScript(editor::IPipelineBuilder* pipelineBuilder, const Guid& scriptGuid, std::list< ResolvedScript >& outScripts)
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

		if (!resolveScript(pipelineBuilder, *i, outScripts))
			return false;
	}

	ResolvedScript rs;
	rs.id = scriptGuid;
	rs.name = scriptInstance->getName();
	rs.script = script;
	outScripts.push_back(rs);

	return true;
}

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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptPipeline", 17, ScriptPipeline, editor::DefaultPipeline)

ScriptPipeline::~ScriptPipeline()
{
	safeDestroy(m_scriptManager);
}

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

	// Create preprocessor.
	m_preprocessor = new Preprocessor();
	
	std::set< std::wstring > definitions = settings->getProperty< PropertyStringSet >(L"ScriptPipeline.PreprocessorDefinitions");
	for (std::set< std::wstring >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
		m_preprocessor->setDefinition(*i);

	// Create debug output directory.
	m_scriptOutputPath = settings->getProperty< PropertyString >(L"ScriptPipeline.OutputPath", L"");
	if (!m_scriptOutputPath.empty())
		FileSystem::getInstance().makeAllDirectories(m_scriptOutputPath);

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
		pipelineDepends->addDependency(*i, editor::PdfUse);

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
	// Create ordered list of dependent scripts.
	std::list< ResolvedScript > scripts;
	if (!resolveScript(pipelineBuilder, outputGuid, scripts))
	{
		log::error << L"Script pipeline failed; unable to resolve script dependencies" << Endl;
		return false;
	}

	// Concate all scripts into a single script; generate a map with line numbers to corresponding source.
	source_map_t sm;
	StringOutputStream ss;
	int32_t line = 0;

	for (std::list< ResolvedScript >::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
	{
		SourceMapping map;
		map.id = i->id;
		map.name = i->name;
		map.line = line;
		sm.push_back(map);

		// Ensure no double character line breaks.
		std::wstring source = i->script->getText();
		source = replaceAll< std::wstring >(source, L"\r\n", L"\n");

		// Execute preprocessor on script.
		std::wstring text;
		if (!m_preprocessor->evaluate(source, text))
		{
			log::error << L"Script pipeline failed; unable to preprocess script" << Endl;
			return false;
		}

		// Count lines.
		StringSplit< std::wstring > split(text, L"\n");
		for (StringSplit< std::wstring >::const_iterator j = split.begin(); j != split.end(); ++j)
			++line;

		// Concatenate scripts.
		ss << text << Endl;
	}

	// Output script source into temp folder; this can be useful for debugging.
	if (!m_scriptOutputPath.empty())
	{
		Ref< IStream > scriptFile = FileSystem::getInstance().open(m_scriptOutputPath + L"/" + outputGuid.format() + L".lua", File::FmWrite);
		if (scriptFile)
		{
			FileOutputStream(scriptFile, new Utf8Encoding()) << ss.str() << Endl;
			scriptFile->close();
			scriptFile = 0;
		}

		Ref< IStream > mapFile = FileSystem::getInstance().open(m_scriptOutputPath + L"/" + outputGuid.format() + L".map", File::FmWrite);
		if (mapFile)
		{
			FileOutputStream fs(mapFile, new Utf8Encoding());
			for (source_map_t::const_iterator i = sm.begin(); i != sm.end(); ++i)
				fs << i->id.format() << L" " << i->name << L" " << i->line << Endl;
			mapFile->close();
			mapFile = 0;
		}
	}

	// Compile script; save binary blobs if possible.
	ErrorCallback errorCallback;
	Ref< IScriptResource > resource = m_scriptManager->compile(outputGuid.format() + L".lua", ss.str(), &sm, &errorCallback);
	if (!resource)
	{
		log::error << L"Script pipeline failed; unable to compile script" << Endl;
		return false;
	}

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
