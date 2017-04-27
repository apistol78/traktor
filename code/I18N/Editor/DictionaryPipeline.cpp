/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "I18N/Dictionary.h"
#include "I18N/Editor/DictionaryAsset.h"
#include "I18N/Editor/DictionaryPipeline.h"
#include "I18N/Editor/IDictionaryFormat.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.i18n.DictionaryPipeline", 0, DictionaryPipeline, editor::DefaultPipeline)

bool DictionaryPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	return editor::DefaultPipeline::create(settings);
}

TypeInfoSet DictionaryPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< DictionaryAsset >());
	return typeSet;
}

bool DictionaryPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< const DictionaryAsset > dictionaryAsset = checked_type_cast< const DictionaryAsset*, false >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), dictionaryAsset->getFileName().getOriginal());
	return true;
}

bool DictionaryPipeline::buildOutput(
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
	Ref< const DictionaryAsset > dictionaryAsset = checked_type_cast< const DictionaryAsset*, false >(sourceAsset);

	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), dictionaryAsset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"Dictionary pipeline failed; unable to open source \"" << dictionaryAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	Ref< Dictionary > dictionary = IDictionaryFormat::readAny(file, dictionaryAsset->getFileName().getExtension(), dictionaryAsset->getKeyColumn(), dictionaryAsset->getTextColumn());
	if (!dictionary)
	{
		log::error << L"Dictionary pipeline failed; unable to read dictionary from \"" << dictionaryAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	safeClose(file);

	return DefaultPipeline::buildOutput(
		pipelineBuilder,
		dependencySet,
		dependency,
		sourceInstance,
		dictionary,
		sourceAssetHash,
		outputPath,
		outputGuid,
		buildParams,
		reason
	);
}

	}
}
