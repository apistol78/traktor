#include "Physics/Editor/ArticulatedEntityPipeline.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "Editor/PipelineManager.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.ArticulatedEntityPipeline", ArticulatedEntityPipeline, world::EntityPipeline)

TypeSet ArticulatedEntityPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< ArticulatedEntityData >());
	return typeSet;
}

bool ArticulatedEntityPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Object* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const ArticulatedEntityData* articulatedEntityData = dynamic_type_cast< const ArticulatedEntityData* >(sourceAsset))
	{
		const RefArray< RigidEntityData >& entities = articulatedEntityData->getEntities();
		for (RefArray< RigidEntityData >::const_iterator i = entities.begin(); i != entities.end(); ++i)
			pipelineManager->addDependency(*i);
	}
	return true;
}

	}
}
