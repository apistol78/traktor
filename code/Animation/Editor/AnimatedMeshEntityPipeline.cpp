#include "Animation/Editor/AnimatedMeshEntityPipeline.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/PoseControllerData.h"
#include "Editor/PipelineManager.h"

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
	editor::PipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const AnimatedMeshEntityData* meshEntityData = dynamic_type_cast< const AnimatedMeshEntityData* >(sourceAsset))
	{
		pipelineManager->addDependency(meshEntityData->getMesh().getGuid());
		pipelineManager->addDependency(meshEntityData->getSkeleton().getGuid());
		pipelineManager->addDependency(meshEntityData->getPoseControllerData());
	}
	return true;
}

	}
}
