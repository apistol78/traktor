#include "Physics/Editor/ArticulatedEntityPipeline.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "World/Entity/EntityInstance.h"
#include "Editor/IPipelineManager.h"

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
	editor::IPipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const ArticulatedEntityData* articulatedEntityData = dynamic_type_cast< const ArticulatedEntityData* >(sourceAsset))
	{
		const RefArray< world::EntityInstance >& instances = articulatedEntityData->getInstances();
		for (RefArray< world::EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
			pipelineManager->addDependency(*i);
	}
	return true;
}

	}
}
