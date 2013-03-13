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
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (!world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outputPath, outputGuid))
		return false;

	if (const RigidEntityData* rigidEntityData = dynamic_type_cast< const RigidEntityData* >(sourceAsset))
		pipelineDepends->addDependency(rigidEntityData->getBodyDesc());

	return true;
}

	}
}
