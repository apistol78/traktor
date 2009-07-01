#include "Animation/Editor/PathEntity/PathEntityPipeline.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "World/Entity/EntityInstance.h"
#include "Editor/PipelineManager.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.PathEntityPipeline", PathEntityPipeline, world::EntityPipeline)

TypeSet PathEntityPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< PathEntityData >());
	return typeSet;
}

bool PathEntityPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const PathEntityData* pathEntityData = dynamic_type_cast< const PathEntityData* >(sourceAsset))
		pipelineManager->addDependency(pathEntityData->getInstance());

	return world::EntityPipeline::buildDependencies(pipelineManager, sourceAsset, outBuildParams);
}

	}
}
