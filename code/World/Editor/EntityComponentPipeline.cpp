/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/DeepClone.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "World/Entity/ScriptComponentData.h"
#include "World/Editor/EntityComponentPipeline.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityComponentPipeline", 0, EntityComponentPipeline, editor::IPipeline)

bool EntityComponentPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void EntityComponentPipeline::destroy()
{
}

TypeInfoSet EntityComponentPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IEntityComponentData >());
	return typeSet;
}

bool EntityComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const ScriptComponentData* scriptComponentData = dynamic_type_cast< const ScriptComponentData* >(sourceAsset))
		pipelineDepends->addDependency(scriptComponentData->m_class, editor::PdfBuild);
	return true;
}

bool EntityComponentPipeline::buildOutput(
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
	T_FATAL_ERROR;
	return false;
}

Ref< ISerializable > EntityComponentPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	return DeepClone(sourceAsset).create();
}

	}
}
