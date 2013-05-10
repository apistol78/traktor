#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Script/IScriptManager.h"
#include "Script/IScriptResource.h"
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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptPipeline", 8, ScriptPipeline, editor::DefaultPipeline)

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

		// Concatenate and count lines.
		StringSplit< std::wstring > split(i->script->getText(), L"\r\n");
		for (StringSplit< std::wstring >::const_iterator j = split.begin(); j != split.end(); ++j)
		{
			ss << *j << Endl;
			++line;
		}
	}

	// Compile script; save binary blobs if possible.
	Ref< IScriptResource > resource = m_scriptManager->compile(outputGuid.format() + L".lua", ss.str(), &sm, 0);
	if (!resource)
	{
		log::error << L"Script pipeline failed; unable to compile script" << Endl;
		return false;
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
