#include "Editor/IPipelineDepends.h"
#include "Physics/BodyDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/Editor/RigidEntityPipeline.h"
#include "Physics/World/RigidEntityData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.RigidEntityPipeline", 0, RigidEntityPipeline, world::EntityPipeline)

TypeInfoSet RigidEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< RigidEntityData >());
	return typeSet;
}

bool RigidEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const RigidEntityData* rigidEntityData = dynamic_type_cast< const RigidEntityData* >(sourceAsset))
	{
		pipelineDepends->addDependency(rigidEntityData->getBodyDesc());

		const world::SpatialEntityData* entityData = rigidEntityData->getEntityData();
		if (entityData)
			pipelineDepends->addDependency(entityData);
	}
	return true;
}

	}
}
