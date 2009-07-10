#include "Spray/Editor/EffectEntityPipeline.h"
#include "Spray/EffectEntityData.h"
#include "Editor/PipelineManager.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.EffectEntityPipeline", EffectEntityPipeline, world::EntityPipeline)

TypeSet EffectEntityPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< EffectEntityData >());
	return typeSet;
}

bool EffectEntityPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const EffectEntityData* effectEntityData = checked_type_cast< const EffectEntityData* >(sourceAsset);
	pipelineManager->addDependency(effectEntityData->getEffect().getGuid());

	return world::EntityPipeline::buildDependencies(pipelineManager, sourceInstance, sourceAsset, outBuildParams);
}

	}
}
