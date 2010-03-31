#include "Editor/IPipelineDepends.h"
#include "Physics/Editor/ArticulatedEntityPipeline.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "Physics/World/RigidEntityData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.ArticulatedEntityPipeline", 0, ArticulatedEntityPipeline, world::EntityPipeline)

TypeInfoSet ArticulatedEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ArticulatedEntityData >());
	return typeSet;
}

bool ArticulatedEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const ArticulatedEntityData* articulatedEntityData = dynamic_type_cast< const ArticulatedEntityData* >(sourceAsset))
	{
		const RefArray< RigidEntityData >& entityData = articulatedEntityData->getEntityData();
		for (RefArray< RigidEntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
			pipelineDepends->addDependency(*i);
	}
	return true;
}

	}
}
