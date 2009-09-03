#include "Spray/Editor/EffectEntityPipeline.h"
#include "Spray/EffectEntityData.h"
#include "Editor/IPipelineManager.h"

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
	editor::IPipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const EffectEntityData* effectEntityData = checked_type_cast< const EffectEntityData* >(sourceAsset);
	pipelineManager->addDependency(effectEntityData->getEffect().getGuid(), true);

	return world::EntityPipeline::buildDependencies(pipelineManager, sourceInstance, sourceAsset, outBuildParams);
}

	}
}
