#include "Database/Instance.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Spray/EffectData.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EmitterData.h"
#include "Spray/SequenceData.h"
#include "Spray/SoundTriggerData.h"
#include "Spray/Editor/EffectPipeline.h"
#include "Spray/Sources/PointSetSourceData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectPipeline", 2, EffectPipeline, editor::IPipeline)

bool EffectPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void EffectPipeline::destroy()
{
}

TypeInfoSet EffectPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EffectData >());
	return typeSet;
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

	const RefArray< EffectLayerData >& layers = effectData->getLayers();
	for (RefArray< EffectLayerData >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		const EmitterData* emitter = (*i)->getEmitter();
		if (emitter)
		{
			pipelineDepends->addDependency(emitter->getShader(), editor::PdfBuild);

			const PointSetSourceData* pointSetSource = dynamic_type_cast< const PointSetSourceData* >(emitter->getSource());
			if (pointSetSource)
				pipelineDepends->addDependency(pointSetSource->getPointSet(), editor::PdfBuild);
		}

		const SequenceData* sequence = (*i)->getSequence();
		if (sequence)
		{
			for (std::vector< SequenceData::Key >::const_iterator i = sequence->getKeys().begin(); i != sequence->getKeys().end(); ++i)
			{
				const SoundTriggerData* soundTrigger = dynamic_type_cast< const SoundTriggerData* >(i->trigger);
				if (soundTrigger)
					pipelineDepends->addDependency(soundTrigger->getSound(), editor::PdfBuild);
			}
		}

		const SoundTriggerData* soundTriggerEnable = dynamic_type_cast< const SoundTriggerData* >((*i)->getTriggerEnable());
		if (soundTriggerEnable)
			pipelineDepends->addDependency(soundTriggerEnable->getSound(), editor::PdfBuild);

		const SoundTriggerData* soundTriggerDisable = dynamic_type_cast< const SoundTriggerData* >((*i)->getTriggerDisable());
		if (soundTriggerDisable)
			pipelineDepends->addDependency(soundTriggerDisable->getSound(), editor::PdfBuild);
	}

	return true;
}

bool EffectPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const EffectData* effectData = checked_type_cast< const EffectData* >(sourceAsset);

	RefArray< EffectLayerData > effectLayers = effectData->getLayers();
	RefArray< EffectLayerData >::iterator i = std::remove(effectLayers.begin(), effectLayers.end(), (EffectLayerData*)0);
	effectLayers.erase(i, effectLayers.end());

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

Ref< ISerializable > EffectPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
