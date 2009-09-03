#include "Animation/Editor/AnimatedMeshEntityPipeline.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/IPoseControllerData.h"
#include "Editor/IPipelineManager.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.AnimatedMeshEntityPipeline", AnimatedMeshEntityPipeline, world::EntityPipeline)

TypeSet AnimatedMeshEntityPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< AnimatedMeshEntityData >());
	return typeSet;
}

bool AnimatedMeshEntityPipeline::buildDependencies(
	editor::IPipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const AnimatedMeshEntityData* meshEntityData = dynamic_type_cast< const AnimatedMeshEntityData* >(sourceAsset))
	{
		pipelineManager->addDependency(meshEntityData->getMesh().getGuid(), true);
		pipelineManager->addDependency(meshEntityData->getSkeleton().getGuid(), true);
		pipelineManager->addDependency(meshEntityData->getPoseControllerData());
	}
	return true;
}

	}
}
