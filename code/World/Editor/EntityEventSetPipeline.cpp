/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "World/EntityEventSetData.h"
#include "World/IEntityEventData.h"
#include "World/Editor/EntityEventSetPipeline.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityEventSetPipeline", 0, EntityEventSetPipeline, editor::IPipeline)

bool EntityEventSetPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void EntityEventSetPipeline::destroy()
{
}

TypeInfoSet EntityEventSetPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EntityEventSetData >());
	return typeSet;
}

bool EntityEventSetPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const EntityEventSetData* eventSetData = checked_type_cast< const EntityEventSetData*, false >(sourceAsset);
	for (std::map< std::wstring, Ref< IEntityEventData > >::const_iterator i = eventSetData->m_eventData.begin(); i != eventSetData->m_eventData.end(); ++i)
		pipelineDepends->addDependency(i->second);
	return true;
}

bool EntityEventSetPipeline::buildOutput(
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
	Ref< EntityEventSetData > eventSet = DeepClone(sourceAsset).create< EntityEventSetData >();
	T_FATAL_ASSERT (eventSet);

	for (std::map< std::wstring, Ref< IEntityEventData > >::iterator i = eventSet->m_eventData.begin(); i != eventSet->m_eventData.end(); )
	{
		std::map< std::wstring, Ref< IEntityEventData > >::iterator at = i++;
		if (!at->second)
			eventSet->m_eventData.erase(at);
	}

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(eventSet);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > EntityEventSetPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
