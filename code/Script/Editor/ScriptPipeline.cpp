/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Preprocessor.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Script/IErrorCallback.h"
#include "Script/IScriptCompiler.h"
#include "Script/ScriptResource.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptPipeline.h"

namespace traktor::script
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

	// Ensure no double character line breaks.
	outSource = replaceAll(outSource, L"\r\n", L"\n");
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
		log::error << L"Script pipeline failed; unable to preprocess script " << scriptGuid.format() << L"." << Endl;
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

bool ScriptPipeline::create(const editor::IPipelineSettings* settings)
{
	// Get implementation type name; return true if no type specified to silence error from pipeline if scripting isn't used.
	std::wstring scriptCompilerTypeName = settings->getPropertyIncludeHash< std::wstring >(L"Editor.ScriptCompilerType");
	if (scriptCompilerTypeName.empty())
		return true;

	// Create script manager instance.
	const TypeInfo* scriptCompilerType = TypeInfo::find(scriptCompilerTypeName.c_str());
	if (!scriptCompilerType)
	{
		log::error << L"Script pipeline failed; no such type \"" << scriptCompilerTypeName << L"\"" << Endl;
		return false;
	}

	m_scriptCompiler = dynamic_type_cast< IScriptCompiler* >(scriptCompilerType->createInstance());
	T_ASSERT(m_scriptCompiler);

	// Create preprocessor.
	m_preprocessor = new Preprocessor();

	if (settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor"))
		m_preprocessor->setDefinition(L"_EDITOR");

	auto definitions = settings->getPropertyIncludeHash< SmallSet< std::wstring > >(L"ScriptPipeline.PreprocessorDefinitions");
	for (const auto& definition : definitions)
		m_preprocessor->setDefinition(definition);

	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");

	return editor::DefaultPipeline::create(settings);
}

TypeInfoSet ScriptPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< Script >();
}

bool ScriptPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	std::wstring source;
	if (!readScript(pipelineDepends, m_assetPath, sourceAsset, source))
		return false;

	// Execute preprocessor on script.
	std::wstring text;
	std::set< std::wstring > usings;
	if (!m_preprocessor->evaluate(source, text, usings))
	{
		log::error << L"Script pipeline failed; unable to preprocess script " << outputGuid.format() << L"." << Endl;
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
				log::error << L"Script pipeline failed; malformed using statement \"" << *i << L"\"." << Endl;
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
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
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
		log::error << L"Script pipeline failed; unable to preprocess script " << outputGuid.format() << L"." << Endl;
		return false;
	}

	// Create ordered list of dependent scripts.
	std::vector< Guid > dependencies;
	if (!flattenDependencies(pipelineBuilder, m_assetPath, m_preprocessor, outputGuid, dependencies))
	{
		log::error << L"Script pipeline failed; unable to resolve script dependencies, in script " << outputGuid.format() << L"." << Endl;
		return false;
	}

	// Ensure current script isn't part of dependencies.
	std::vector< Guid >::iterator i = std::find(dependencies.begin(), dependencies.end(), outputGuid);
	if (i != dependencies.end())
		dependencies.erase(i);

	// Compile script; save binary blobs if possible.
	ErrorCallback errorCallback;
	Ref< IScriptBlob > blob = m_scriptCompiler->compile(outputGuid.format(), text, &errorCallback);
	if (!blob)
	{
		log::error << L"Script pipeline failed; unable to compile script " << outputGuid.format() << L"." << Endl;
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
		outputPath,
		outputGuid,
		buildParams,
		reason
	);
}

}
