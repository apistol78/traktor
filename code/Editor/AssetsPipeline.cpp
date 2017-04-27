/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/Assets.h"
#include "Editor/AssetsPipeline.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.AssetsPipeline", 1, AssetsPipeline, IPipeline)

AssetsPipeline::AssetsPipeline()
:	m_editorDeploy(false)
{
}

bool AssetsPipeline::create(const IPipelineSettings* settings)
{
	m_editorDeploy = settings->getProperty< bool >(L"Pipeline.EditorDeploy", false);
	return true;
}

void AssetsPipeline::destroy()
{
}

TypeInfoSet AssetsPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Assets >());
	return typeSet;
}

bool AssetsPipeline::buildDependencies(
	IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const Assets* assets = dynamic_type_cast< const Assets* >(sourceAsset))
	{
		for (std::vector< Assets::Dependency >::const_iterator i = assets->m_dependencies.begin(); i != assets->m_dependencies.end(); ++i)
		{
			if (!i->editorDeployOnly || m_editorDeploy)
				pipelineDepends->addDependency(i->id, editor::PdfBuild);
		}
	}
	return true;
}

bool AssetsPipeline::buildOutput(
	IPipelineBuilder* pipelineBuilder,
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
	return true;
}

Ref< ISerializable > AssetsPipeline::buildOutput(
	IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
