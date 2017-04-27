/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Editor/StagePipeline.h"
#include "Amalgam/Game/Engine/AudioLayerData.h"
#include "Amalgam/Game/Engine/FlashLayerData.h"
#include "Amalgam/Game/Engine/SparkLayerData.h"
#include "Amalgam/Game/Engine/StageData.h"
#include "Amalgam/Game/Engine/VideoLayerData.h"
#include "Amalgam/Game/Engine/WorldLayerData.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{
	
Ref< StageData > flattenInheritance(editor::IPipelineBuilder* pipelineBuilder, const StageData* stageData)
{
	Ref< StageData > stageDataOut = DeepClone(checked_type_cast< const StageData*, false >(stageData)).create< StageData >();
	T_ASSERT (stageDataOut);

	if (stageData->getInherit().isNotNull())
	{
		Ref< const StageData > downStageData = pipelineBuilder->getObjectReadOnly< StageData >(stageData->getInherit());
		if (!downStageData)
			return 0;

		Ref< StageData > downStageDataFlatten = flattenInheritance(pipelineBuilder, downStageData);
		if (!downStageData)
			return 0;

		// Append layers.
		RefArray< LayerData > downStageLayers = downStageDataFlatten->getLayers();
		if (!downStageLayers.empty())
		{
			RefArray< LayerData > layers = stageDataOut->getLayers();
			layers.insert(layers.end(), downStageLayers.begin(), downStageLayers.end());
			stageDataOut->setLayers(layers);
		}

		// Replace script.
		if (stageDataOut->getClass().isNull())
			stageDataOut->setClass(downStageDataFlatten->getClass());
		if (stageDataOut->getScript().isNull())
			stageDataOut->setScript(downStageDataFlatten->getScript());

		// Replace shader fade.
		if (stageDataOut->getShaderFade().isNull())
		{
			stageDataOut->setShaderFade(downStageDataFlatten->getShaderFade());
			stageDataOut->setFadeRate(downStageDataFlatten->getFadeRate());
		}

		// Merge transitions.
		std::map< std::wstring, Guid > transitions = downStageDataFlatten->getTransitions();
		transitions.insert(stageDataOut->getTransitions().begin(), stageDataOut->getTransitions().end());
		stageDataOut->setTransitions(transitions);

		// Replace resource bundle.
		if (stageDataOut->getResourceBundle().isNull())
			stageDataOut->setResourceBundle(downStageDataFlatten->getResourceBundle());

		if (stageDataOut->getProperties() && downStageData->getProperties())
		{
			Ref< const PropertyGroup > mergedProperties = downStageData->getProperties()->mergeReplace(stageDataOut->getProperties());
			stageDataOut->setProperties(mergedProperties);
		}
		else if (downStageData->getProperties())
			stageDataOut->setProperties(downStageData->getProperties());
	}

	return stageDataOut;
}
		
		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.StagePipeline", 6, StagePipeline, editor::DefaultPipeline)

TypeInfoSet StagePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< StageData >());
	return typeSet;
}

bool StagePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const StageData* stageData = checked_type_cast< const StageData*, false >(sourceAsset);

	pipelineDepends->addDependency(stageData->m_inherit, editor::PdfBuild);
	pipelineDepends->addDependency(stageData->m_class, editor::PdfBuild);
	pipelineDepends->addDependency(stageData->m_script, editor::PdfBuild);
	pipelineDepends->addDependency(stageData->m_shaderFade, editor::PdfBuild | editor::PdfResource);

	for (std::map< std::wstring, Guid >::const_iterator i = stageData->m_transitions.begin(); i != stageData->m_transitions.end(); ++i)
		pipelineDepends->addDependency(i->second, editor::PdfBuild);

	for (RefArray< LayerData >::const_iterator i = stageData->m_layers.begin(); i != stageData->m_layers.end(); ++i)
	{
		if (const AudioLayerData* audioLayer = dynamic_type_cast< const AudioLayerData* >(*i))
			pipelineDepends->addDependency(audioLayer->m_sound, editor::PdfBuild);
		else if (const FlashLayerData* flashLayer = dynamic_type_cast< const FlashLayerData* >(*i))
		{
			pipelineDepends->addDependency(flashLayer->m_movie, editor::PdfBuild);
			for (std::map< std::wstring, resource::Id< flash::FlashMovie > >::const_iterator i = flashLayer->m_externalMovies.begin(); i != flashLayer->m_externalMovies.end(); ++i)
				pipelineDepends->addDependency(i->second, editor::PdfBuild);
			pipelineDepends->addDependency(flashLayer->m_imageProcess, editor::PdfBuild);
		}
		else if (const SparkLayerData* sparkLayer = dynamic_type_cast< const SparkLayerData* >(*i))
		{
			pipelineDepends->addDependency(sparkLayer->m_sprite, editor::PdfBuild);
			pipelineDepends->addDependency(sparkLayer->m_imageProcess, editor::PdfBuild);
		}
		else if (const VideoLayerData* videoLayer = dynamic_type_cast< const VideoLayerData* >(*i))
		{
			pipelineDepends->addDependency(videoLayer->m_video, editor::PdfBuild);
			pipelineDepends->addDependency(videoLayer->m_shader, editor::PdfBuild | editor::PdfResource);
		}
		else if (const WorldLayerData* worldLayer = dynamic_type_cast< const WorldLayerData* >(*i))
		{
			pipelineDepends->addDependency(worldLayer->m_scene, editor::PdfBuild);
			for (std::map< std::wstring, resource::Id< world::EntityData > >::const_iterator j = worldLayer->m_entities.begin(); j != worldLayer->m_entities.end(); ++j)
				pipelineDepends->addDependency(j->second, editor::PdfBuild);
		}
	}

	pipelineDepends->addDependency(stageData->m_resourceBundle, editor::PdfBuild);
	return true;
}

bool StagePipeline::buildOutput(
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
	Ref< StageData > stageData = flattenInheritance(pipelineBuilder, checked_type_cast< const StageData*, false >(sourceAsset));
	if (!stageData)
		return false;

	if (stageData->m_name.empty())
		stageData->m_name = sourceInstance->getName();
	
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(stageData);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

	}
}
