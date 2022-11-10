/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Runtime/Editor/StagePipeline.h"
#include "Runtime/Engine/AudioLayerData.h"
#include "Runtime/Engine/ScreenLayerData.h"
#include "Runtime/Engine/StageData.h"
#include "Runtime/Engine/VideoLayerData.h"
#include "Runtime/Engine/WorldLayerData.h"

namespace traktor
{
	namespace runtime
	{
		namespace
		{

Ref< StageData > flattenInheritance(editor::IPipelineBuilder* pipelineBuilder, const StageData* stageData)
{
	Ref< StageData > stageDataOut = DeepClone(checked_type_cast< const StageData*, false >(stageData)).create< StageData >();
	T_ASSERT(stageDataOut);

	if (stageData->getInherit().isNotNull())
	{
		Ref< const StageData > downStageData = pipelineBuilder->getObjectReadOnly< StageData >(stageData->getInherit());
		if (!downStageData)
			return nullptr;

		Ref< StageData > downStageDataFlatten = flattenInheritance(pipelineBuilder, downStageData);
		if (!downStageData)
			return nullptr;

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

		// Replace shader fade.
		if (stageDataOut->getShaderFade().isNull())
		{
			stageDataOut->setShaderFade(downStageDataFlatten->getShaderFade());
			stageDataOut->setFadeOutUpdate(downStageDataFlatten->getFadeOutUpdate());
			stageDataOut->setFadeRate(downStageDataFlatten->getFadeRate());
		}

		// Merge transitions.
		SmallMap< std::wstring, Guid > transitions = downStageDataFlatten->getTransitions();
		transitions.insert(stageDataOut->getTransitions().begin(), stageDataOut->getTransitions().end());
		stageDataOut->setTransitions(transitions);

		// Replace resource bundle.
		if (stageDataOut->getResourceBundle().isNull())
			stageDataOut->setResourceBundle(downStageDataFlatten->getResourceBundle());

		if (stageDataOut->getProperties() && downStageData->getProperties())
		{
			Ref< const PropertyGroup > mergedProperties = downStageData->getProperties()->merge(stageDataOut->getProperties(), PropertyGroup::MmReplace);
			stageDataOut->setProperties(mergedProperties);
		}
		else if (downStageData->getProperties())
			stageDataOut->setProperties(downStageData->getProperties());
	}

	return stageDataOut;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.StagePipeline", 10, StagePipeline, editor::DefaultPipeline)

TypeInfoSet StagePipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		StageData,
		LayerData
	>();
}

bool StagePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const StageData* stageData = dynamic_type_cast< const StageData* >(sourceAsset))
	{
		pipelineDepends->addDependency(stageData->m_inherit, editor::PdfUse);
		pipelineDepends->addDependency(stageData->m_class, editor::PdfBuild);
		pipelineDepends->addDependency(stageData->m_shaderFade, editor::PdfBuild | editor::PdfResource);

		for (auto i = stageData->m_transitions.begin(); i != stageData->m_transitions.end(); ++i)
			pipelineDepends->addDependency(i->second, editor::PdfBuild);

		for (const auto layerData : stageData->m_layers)
			pipelineDepends->addDependency(layerData);

		pipelineDepends->addDependency(stageData->m_resourceBundle, editor::PdfBuild);
		return true;
	}
	else if (const LayerData* layerData = dynamic_type_cast< const LayerData* >(sourceAsset))
	{
		if (const AudioLayerData* audioLayer = dynamic_type_cast< const AudioLayerData* >(layerData))
			pipelineDepends->addDependency(audioLayer->m_sound, editor::PdfBuild);
		else if (const ScreenLayerData* screenLayer = dynamic_type_cast< const ScreenLayerData* >(layerData))
			pipelineDepends->addDependency(screenLayer->m_shader, editor::PdfBuild);
		else if (const VideoLayerData* videoLayer = dynamic_type_cast< const VideoLayerData* >(layerData))
		{
			pipelineDepends->addDependency(videoLayer->m_video, editor::PdfBuild);
			pipelineDepends->addDependency(videoLayer->m_shader, editor::PdfBuild | editor::PdfResource);
		}
		else if (const WorldLayerData* worldLayer = dynamic_type_cast< const WorldLayerData* >(layerData))
			pipelineDepends->addDependency(worldLayer->m_scene, editor::PdfBuild);
		else
		{
			log::error << L"Unknown layer data type \"" << type_name(layerData) << L"\"; missing pipeline?" << Endl;
			return false;
		}
		return true;
	}
	return false;
}

bool StagePipeline::buildOutput(
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
	if (const StageData* immutableStageData = dynamic_type_cast< const StageData* >(sourceAsset))
	{
		Ref< StageData > stageData = flattenInheritance(pipelineBuilder, immutableStageData);
		if (!stageData)
			return false;

		if (stageData->m_name.empty())
			stageData->m_name = sourceInstance->getName();

		Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
		if (!outputInstance)
		{
			log::error << L"Stage pipeline failed; unable to create output instance." << Endl;
			return false;
		}

		outputInstance->setObject(stageData);

		if (!outputInstance->commit())
		{
			log::error << L"Stage pipeline failed; unable to commit output instance." << Endl;
			return false;
		}
	}
	return true;
}

	}
}
