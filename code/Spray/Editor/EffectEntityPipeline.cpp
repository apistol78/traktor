#include "Spray/Editor/EffectEntityPipeline.h"
#include "Spray/EffectEntityData.h"
#include "Spray/SoundEventData.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectEntityPipeline", 1, EffectEntityPipeline, world::EntityPipeline)

TypeInfoSet EffectEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SoundEventData >());
	typeSet.insert(&type_of< EffectEntityData >());
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
	if (const SoundEventData* soundEventData = dynamic_type_cast< const SoundEventData* >(sourceAsset))
		pipelineDepends->addDependency(soundEventData->m_sound, editor::PdfBuild | editor::PdfResource);
	else if (const EffectEntityData* effectEntityData = dynamic_type_cast< const EffectEntityData* >(sourceAsset))
		pipelineDepends->addDependency(effectEntityData->getEffect(), editor::PdfBuild | editor::PdfResource);

	return true;
}

Ref< ISerializable > EffectEntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	if (const EffectEntityData* effectEntityData = dynamic_type_cast< const EffectEntityData* >(sourceAsset))
	{
		if (effectEntityData->getEffect().isNull())
			return 0;
	}

	return world::EntityPipeline::buildOutput(pipelineBuilder, sourceAsset);
}

	}
}
