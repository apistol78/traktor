/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepHash.h"
#include "Database/Instance.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Spray/EffectData.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EmitterData.h"
#include "Spray/SequenceData.h"
#include "Spray/TrailData.h"
#include "Spray/Editor/EffectPipeline.h"
#include "Spray/Sources/PointSetSourceData.h"
#include "World/IEntityEventData.h"

namespace traktor::spray
{
	namespace
	{

void buildEffectDependencies(editor::IPipelineDepends* pipelineDepends, const EffectData* effectData)
{
	for (auto layer : effectData->getLayers())
	{
		const EmitterData* emitter = layer->getEmitter();
		if (emitter)
		{
			pipelineDepends->addDependency(emitter->getShader(), editor::PdfBuild | editor::PdfResource);
			pipelineDepends->addDependency(emitter->getMesh(), editor::PdfBuild | editor::PdfResource);

			if (emitter->getEffect())
				buildEffectDependencies(pipelineDepends, emitter->getEffect());

			const PointSetSourceData* pointSetSource = dynamic_type_cast< const PointSetSourceData* >(emitter->getSource());
			if (pointSetSource)
				pipelineDepends->addDependency(pointSetSource->getPointSet(), editor::PdfBuild | editor::PdfResource);
		}

		const TrailData* trail = layer->getTrail();
		if (trail)
			pipelineDepends->addDependency(trail->getShader(), editor::PdfBuild | editor::PdfResource);

		const SequenceData* sequence = layer->getSequence();
		if (sequence)
		{
			for (auto key : sequence->getKeys())
				pipelineDepends->addDependency(key.event);
		}

		pipelineDepends->addDependency(layer->getTriggerEnable());
		pipelineDepends->addDependency(layer->getTriggerDisable());
	}
}

bool effectLayerPred(EffectLayerData* layerData)
{
	if (!layerData)
		return true;

	bool haveEmitter = false;
	if (layerData->getEmitter() != nullptr)
	{
		if (
			layerData->getEmitter()->getSource() != nullptr &&
			(
				layerData->getEmitter()->getShader() ||
				layerData->getEmitter()->getMesh() ||
				layerData->getEmitter()->getEffect()
			)
		)
			haveEmitter = true;
	}

	bool haveTrail = false;
	if (layerData->getTrail() != nullptr)
	{
		if (layerData->getTrail()->getShader())
			haveTrail = true;
	}

	bool haveSequence = false;
	if (layerData->getSequence() != nullptr)
	{
		if (!layerData->getSequence()->getKeys().empty())
			haveSequence = true;
	}

	bool haveTrigger = false;
	if (layerData->getTriggerEnable() != nullptr || layerData->getTriggerDisable() != nullptr)
		haveTrigger = true;

	if (haveEmitter || haveTrail || haveSequence || haveTrigger)
		return false;

	return true;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectPipeline", 4, EffectPipeline, editor::IPipeline)

bool EffectPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void EffectPipeline::destroy()
{
}

TypeInfoSet EffectPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< EffectData >();
}

bool EffectPipeline::shouldCache() const
{
	return false;
}

uint32_t EffectPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool EffectPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const EffectData* effectData = checked_type_cast< const EffectData* >(sourceAsset);
	buildEffectDependencies(pipelineDepends, effectData);
	return true;
}

bool EffectPipeline::buildOutput(
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
	const EffectData* effectData = checked_type_cast< const EffectData* >(sourceAsset);

	RefArray< EffectLayerData > effectLayers = effectData->getLayers();

	auto it = std::remove_if(effectLayers.begin(), effectLayers.end(), effectLayerPred);
	effectLayers.erase(it, effectLayers.end());

	Ref< EffectData > outEffectData = new EffectData(
		effectData->getDuration(),
		effectData->getLoopStart(),
		effectData->getLoopEnd(),
		effectLayers
	);

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
		return false;

	instance->setObject(outEffectData);

	return instance->commit();
}

Ref< ISerializable > EffectPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

}
