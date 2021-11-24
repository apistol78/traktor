#include "Editor/IPipelineDepends.h"
#include "Spray/EffectComponentData.h"
#include "Spray/SoundEventData.h"
#include "Spray/SpawnEffectEventData.h"
#include "Spray/Editor/EffectEntityPipeline.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectEntityPipeline", 1, EffectEntityPipeline, world::EntityPipeline)

TypeInfoSet EffectEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< EffectComponentData >();
	typeSet.insert< SoundEventData >();
	typeSet.insert< SpawnEffectEventData >();
	return typeSet;
}

bool EffectEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (auto effectComponentData = dynamic_type_cast< const EffectComponentData* >(sourceAsset))
		pipelineDepends->addDependency(effectComponentData->getEffect(), editor::PdfBuild | editor::PdfResource);
	else if (auto soundEventData = dynamic_type_cast< const SoundEventData* >(sourceAsset))
		pipelineDepends->addDependency(soundEventData->m_sound, editor::PdfBuild | editor::PdfResource);
	else if (auto spawnEventData = dynamic_type_cast< const SpawnEffectEventData* >(sourceAsset))
		pipelineDepends->addDependency(spawnEventData->getEffect(), editor::PdfBuild | editor::PdfResource);

	return true;
}

Ref< ISerializable > EffectEntityPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	if (auto soundEventData = dynamic_type_cast< const SoundEventData* >(sourceAsset))
	{
		if (soundEventData->m_sound.isNull())
			return nullptr;
	}
	else if (auto spawnEventData = dynamic_type_cast< const SpawnEffectEventData* >(sourceAsset))
	{
		if (spawnEventData->getEffect().isNull())
			return nullptr;
	}

	return world::EntityPipeline::buildProduct(pipelineBuilder, sourceInstance, sourceAsset, buildParams);
}

	}
}
